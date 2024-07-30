-- app.lua
dofile("utils.lua")
dofile("templating.lua")


app.get("/hello", function(req)
    local res = {}
    res.body = "Hello World"
    res.status = 200
    res.headers = {
        ["Content-Type"] = "text/plain"
    }
    return res
end)

app.get("/user/:id", function(req)
    printTable(req)
    -- print(req.path)
    local id = string.match(req.path, "/user/(.+)")

    local res = {}
    res.body = "Hello User " .. id
    res.status = 200
    res.headers = {
        ["Content-Type"] = "text/plain"
    }
    return res
end)

app.get("/template", function(req)
    local context = {
        title = "My Page",
        show_items = true,
        items = {
            {name = "Item 1", price = "10.00"},
            {name = "Item 2", price = "15.00"},
            {name = "Item 3", price = "20.00"}
        },
        loop_count = 3
    }

    local res = {}
    res.body = render_template("test.html", context)
    res.status = 200
    res.headers = {
        ["Content-Type"] = "text/html"
    }
    return res
end)
