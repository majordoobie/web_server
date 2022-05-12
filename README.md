# ashti

Ashti is a webserver written in C. It is able to accept multiple concurrent connections
but it does this using forking which is definitely not the beset approach. But 
with the 48 hour time constraint for the project we chose forking as our solution.

Every connection made to the server will attempt to server the client the resource
they requested using the RFC2616 (HTTP) protocol. The resource they request is checked
to make sure that the resource is only located in the sandbox directory. 

An example of how a resource is provided is: 

| CMD                     | Task                                                                                                              | Return                                                                                                                                   | HTML Code                      |
|-------------------------|-------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------|
| GET /                   | - Do not search<br /> - Auto return $WEB/index.html                                                               | $WEB/index.html                                                                                                                          | 200                            |
| GET /index.html         | - Do a `search` in `$WEB`                                                                                         | $WEB/index.html                                                                                                                          | 200                            |
| GET /../index.html      | - Do a `search` in `$WEB`<br /> - Search should resolve to a path OUTSIDE of the `$WEB` sandbox which is an error | NULL                                                                                                                                     | 404                            |
| GET /cgi-bin/index.html | - Do a search of `$CGI` if found, parse and return                                                                | - IF exist & executable return result of executable<br /> - IF exists & NOT executable, return `NULL`<br /> - IF no exist, return `NULL` | - 200 <br /> - 403<br /> - 404 |

**Legend** 

| Key      | Meaning                                               |
|----------|-------------------------------------------------------|
| `$WEB`   | The web sandbox directory where HTML files are stored |
| `$CGI`   | CGI directory where things CAN execute and return     |
| `search` | The action of searching for a resource file           |


## How to run

Running requires a Linux host machine with `make` available. Running make will 
create the `ashti` binary in the current working directory of the repo

```bash
./ashti -h
Invalid argument supplied. The server must receive a mandatory file path to its root folder. 
Additionally, a -c option may be supplied to modify for how many connections it should listen to before it errors. 
A value of 0 will listen for ever.

Examples:
        ./ashti /path/to/www_root
        ./ashti www_root -c 4
```


## Errors
Ashti was tested with the `check.h` framework. I have tried to create instructions 
for the framework but it looks like the latest version of `chech.h` does not support
the pattern I used to set up the tests. I tried to download different versions but 
none of them worked. I do not know what version was used in class, this project
was also compiled on their VM and I no longer have access to their VM to see
the version of `check.h` was used. I have disabled the tests to avoid compilation 
errors. 




