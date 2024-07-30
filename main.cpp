#include "httplib.h"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <luajit.h>
}

#include <iostream>
#include <string>
#include <regex>

httplib::Server svr;

void report_errors(lua_State *L, int status) {
    if (status != 0) {
        std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1); // remove error message
    }
}

int app_post(lua_State *L) {
    // Check if the number of arguments is correct
    if (lua_gettop(L) < 2) {
        luaL_error(L, "Expected at least 2 arguments: path and callback function");
        return 0;
    }

    // Get the path argument
    const char *path = lua_tostring(L, 1);
    if (!path) {
        luaL_error(L, "Path must be a string");
        return 0;
    }

    // Ensure the second argument is a function
    if (!lua_isfunction(L, 2)) {
        luaL_error(L, "Callback must be a function");
        return 0;
    }

    // Store the Lua function in the registry
    lua_pushvalue(L, 2);  // push the function onto the stack
    int callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);  // get a reference to the function

    // Set up the HTTP route
    svr.Post(path, [L, callback_ref](const httplib::Request &req, httplib::Response &res) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, callback_ref);  // get the callback function

        // Create request table
        lua_newtable(L);
        lua_pushstring(L, req.method.c_str());
        lua_setfield(L, -2, "method");
        lua_pushstring(L, req.path.c_str());
        lua_setfield(L, -2, "path");

        // Create headers table
        lua_newtable(L);
        for (const auto &header : req.headers) {
            lua_pushstring(L, header.second.c_str());
            lua_setfield(L, -2, header.first.c_str());
        }
        lua_setfield(L, -2, "headers");

        lua_pushstring(L, req.body.c_str());
        lua_setfield(L, -2, "body");

        // Capture path parameters using regex
        std::regex path_regex(req.path);
        std::smatch matches;
        std::regex_search(req.path, matches, path_regex);

        // Push path parameters as separate arguments
        int num_params = 0;
        for (size_t i = 1; i < matches.size(); ++i) {
            lua_pushstring(L, matches[i].str().c_str());
            num_params++;
        }

        // Call the Lua function with the request table and path parameters
        if (lua_pcall(L, 1 + num_params, 1, 0) != LUA_OK) {
            std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);  // remove error message
            res.status = 500;
            res.set_content("Internal Server Error", "text/plain");
            return;
        }

        std::string content_type = "text/plain";
        // Process the response table returned by the Lua function
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "status");
            res.status = luaL_optinteger(L, -1, 200);
            lua_pop(L, 1);

            lua_getfield(L, -1, "headers");
            if (lua_istable(L, -1)) {
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    const char *key = lua_tostring(L, -2);
                    const char *value = lua_tostring(L, -1);
                    res.set_header(key, value);
                    lua_pop(L, 1);

                    if (strcmp(key, "Content-Type") == 0) {
                        content_type = value;
                    }
                }
            }
            lua_pop(L, 1);

            lua_getfield(L, -1, "body");
            if (lua_isstring(L, -1)) {
                res.set_content(lua_tostring(L, -1), content_type.c_str());
            } else {
                res.set_content("", "text/plain");
            }
            lua_pop(L, 1);
        } else {
            res.status = 500;
            res.set_content("Invalid response from Lua function", "text/plain");
        }

        lua_pop(L, 1);  // remove the response table
    });

    return 0;
}

// Function to register routes in the HTTP server
int app_get(lua_State *L) {
    // Check if the number of arguments is correct
    if (lua_gettop(L) < 2) {
        luaL_error(L, "Expected at least 2 arguments: path and callback function");
        return 0;
    }

    // Get the path argument
    const char *path = lua_tostring(L, 1);
    if (!path) {
        luaL_error(L, "Path must be a string");
        return 0;
    }

    // Ensure the second argument is a function
    if (!lua_isfunction(L, 2)) {
        luaL_error(L, "Callback must be a function");
        return 0;
    }

    // Store the Lua function in the registry
    lua_pushvalue(L, 2);  // push the function onto the stack
    int callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);  // get a reference to the function

    // Set up the HTTP route
    svr.Get(path, [L, callback_ref](const httplib::Request &req, httplib::Response &res) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, callback_ref);  // get the callback function

        // Create request table
        lua_newtable(L);
        lua_pushstring(L, req.method.c_str());
        lua_setfield(L, -2, "method");
        lua_pushstring(L, req.path.c_str());
        lua_setfield(L, -2, "path");

        // Create headers table
        lua_newtable(L);
        for (const auto &header : req.headers) {
            lua_pushstring(L, header.second.c_str());
            lua_setfield(L, -2, header.first.c_str());
        }
        lua_setfield(L, -2, "headers");

        lua_pushstring(L, req.body.c_str());
        lua_setfield(L, -2, "body");

        // Capture path parameters using regex
        std::regex path_regex(req.path);
        std::smatch matches;
        std::regex_search(req.path, matches, path_regex);

        // Push path parameters as separate arguments
        int num_params = 0;
        for (size_t i = 1; i < matches.size(); ++i) {
            lua_pushstring(L, matches[i].str().c_str());
            num_params++;
        }

        // Call the Lua function with the request table and path parameters
        if (lua_pcall(L, 1 + num_params, 1, 0) != LUA_OK) {
            std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);  // remove error message
            res.status = 500;
            res.set_content("Internal Server Error", "text/plain");
            return;
        }


        std::string content_type = "text/plain";
        // Process the response table returned by the Lua function
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "status");
            res.status = luaL_optinteger(L, -1, 200);
            lua_pop(L, 1);

            lua_getfield(L, -1, "headers");
            if (lua_istable(L, -1)) {
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    const char *key = lua_tostring(L, -2);
                    const char *value = lua_tostring(L, -1);
                    res.set_header(key, value);
                    lua_pop(L, 1);

                    if (strcmp(key, "Content-Type") == 0) {
                        content_type = value;
                    }
                }
            }
            lua_pop(L, 1);

            lua_getfield(L, -1, "body");
            if (lua_isstring(L, -1)) {
                res.set_content(lua_tostring(L, -1), content_type.c_str());
            } else {
                res.set_content("", "text/plain");
            }
            lua_pop(L, 1);
        } else {
            res.status = 500;
            res.set_content("Invalid response from Lua function", "text/plain");
        }

        lua_pop(L, 1);  // remove the response table
    });

    return 0;
}

int main(int argc, char **argv) {
    int port = argc > 1 ? std::stoi(argv[1]) : 8080;

    lua_State *L = luaL_newstate(); // create a new lua state
    luaL_openlibs(L); // open lua standard libraries

    // Create app table
    lua_newtable(L);
    lua_setglobal(L, "app");

    // Set the get function
    lua_getglobal(L, "app");
    lua_pushcfunction(L, app_get);
    lua_setfield(L, -2, "get");

    // Load and run the Lua script file
    int status = luaL_loadfile(L, "app.lua");
    if (status == 0) {
        status = lua_pcall(L, 0, LUA_MULTRET, 0);
    }
    report_errors(L, status);

    std::cout << "Server started on localhost on port " << port << std::endl;
    svr.listen("localhost", port);
    lua_close(L);

    return 0;
}
