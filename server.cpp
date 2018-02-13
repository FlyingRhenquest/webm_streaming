/**
 * Copyright 2018 Bruce Ide
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * A quick and dirty REST server for posting images to files. It's
 * probably best that this server never be run on a network-facing
 * interface.
 */

#include <boost/log/trivial.hpp>
#include <base64_default_rfc4648.hpp>
#include <served/served.hpp>
#include <vector>

/*
 * I'm doing some old school C IO in here, as it's generally more
 * painful in C++.
 */

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Return a file not found through the served::response 
void file_not_found(served::response &res)
{
  res.set_status(404);
  res << "File not found";
}

int main(int argc, char *argv[])
{
  served::multiplexer mux;
  
  mux.handle("/image/{filename}")
    .get([](served::response &res, const served::request &req) {
	std::string filename("images/");
	filename.append(req.params["filename"]);
	struct stat file_status;
	if (stat(filename.c_str(), &file_status) != 0 || file_status.st_size == 0) {
	  file_not_found(res);
	} else {
	  FILE *f = fopen(filename.c_str(), "rb");
	  if (nullptr == f) {
	    file_not_found(res);
	  } else {
	    // Read the whole file into an allocated buffer and
	    // send it to the user
	    
	    char *buffer = (char *) malloc(file_status.st_size * sizeof(uint8_t));
	    fread(buffer, sizeof(uint8_t), file_status.st_size, f);
	    fclose(f);
	    res.set_status(200);
	    res.set_header("content-type", "image/jpeg");
	    // Blah served seems to explicitly want a string here :/
	    std::string s_buf;
	    s_buf.append(buffer, (size_t) file_status.st_size);
	    res.set_body(s_buf);
	    free(buffer);
	    fclose(f);
	  }
	}
      })
    .post([](served::response &res, const served::request &req) {
	// As-written, post will overwrite existing files if they're
	// posted again. It should really check and refuse.
	std::string filename("images/");
	filename.append(req.params["filename"]);
	FILE *f = fopen(filename.c_str(), "wb");
	if (nullptr == f) {
	  file_not_found(res);
	} else {
	  std::vector<uint8_t> buffer = base64::decode(req.body());
	  fwrite((void *) &buffer[0], sizeof(uint8_t), buffer.size(), f);
	  fclose(f);
	  res.set_status(200);
	  res << "OK";
	}
      });

    // Top level returns list of files in images directory as a
  // JSON array of strings
  mux.handle("/images")
    .get([](served::response &res, const served::request &req) {
	DIR *d = opendir("images");
	if (nullptr == d) {
	  file_not_found(res);
	} else {	 
	  res.set_status(200);
	  res.set_header("content-type", "application/json");
	  res << "[";
	  struct dirent *entry;
	  bool first = true;
	  while(nullptr != (entry = readdir(d))) {
	    // Don't send files that start with '.'
	    if ('.' != *entry->d_name) {
	      if (!first) {
		res << ", ";
	      } else {
		first = false;
	      }
	      res << "\"" << entry->d_name << "\"";
	    }
	  }
	  closedir(d);
	  res << "]";
	}
      });

  // Set up and start server
  served::net::server server("127.0.0.1", "8080", mux);
  server.run(5);
}
