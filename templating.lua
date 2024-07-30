function render_template(template, context)
    -- Read the template file
    local file = io.open("./templates/" .. template, "r")
    if not file then error("Template file not found") end
    local content = file:read("*a")
    file:close()

    -- Function to replace variables
    local function replace_vars(str, ctx)
        return (string.gsub(str, "{{(.-)}}", function(key)
            key = key:match("^%s*(.-)%s*$")  -- trim spaces
            return ctx[key] or ("{{" .. key .. "}}")
        end))
    end

    -- Function to handle loops over tables
    local function handle_for_in_loops(str, ctx)
        return (string.gsub(str, "{%% for (.-) in (.-) %%}(.-){%% endfor %%}", function(var, tbl, inner)
            var = var:match("^%s*(.-)%s*$")  -- trim spaces
            tbl = tbl:match("^%s*(.-)%s*$")  -- trim spaces
            local list = ctx[tbl]
            if type(list) ~= "table" then return "" end

            local result = {}
            for _, item in ipairs(list) do
                local loop_ctx = {}
                if type(item) == "table" then
                    for k, v in pairs(item) do
                        loop_ctx[k] = v
                    end
                else
                    loop_ctx[var] = item
                end
                table.insert(result, replace_vars(inner, loop_ctx))
            end
            return table.concat(result)
        end))
    end

    -- Function to handle linear for loops
    local function handle_for_loops(str, ctx)
        return (string.gsub(str, "{%% for (.-) = (.-), (.-), (.-) %%}(.-){%% endfor %%}", function(var, start_expr, end_expr, step_expr, inner)
            var = var:match("^%s*(.-)%s*$")  -- trim spaces
            local start_val = tonumber(ctx[start_expr] or start_expr)
            local end_val = tonumber(ctx[end_expr] or end_expr)
            local step_val = tonumber(ctx[step_expr] or step_expr)
            local result = {}

            for i = start_val, end_val, step_val do
                local loop_ctx = {[var] = i}
                table.insert(result, replace_vars(inner, loop_ctx))
            end
            return table.concat(result)
        end))
    end

    -- Function to handle while loops
    local function handle_while_loops(str, ctx)
        return (string.gsub(str, "{%% while (.-) %%}(.-){%% endwhile %%}", function(condition, inner)
            condition = condition:match("^%s*(.-)%s*$")  -- trim spaces
            local result = {}
            local env = setmetatable(ctx, {__index = _G})

            while load("return " .. condition, "condition", "t", env)() do
                table.insert(result, replace_vars(inner, ctx))
            end
            return table.concat(result)
        end))
    end

    -- Function to handle repeat until loops
    local function handle_repeat_until_loops(str, ctx)
        return (string.gsub(str, "{%% repeat %%}(.-){%% until (.-) %%}", function(inner, condition)
            condition = condition:match("^%s*(.-)%s*$")  -- trim spaces
            local result = {}
            local env = setmetatable(ctx, {__index = _G})

            repeat
                table.insert(result, replace_vars(inner, ctx))
            until load("return " .. condition, "condition", "t", env)()
            return table.concat(result)
        end))
    end

    -- Function to handle if statements
    local function handle_if_statements(str, ctx)
        return (string.gsub(str, "{%% if (.-) %%}(.-){%% else %%}(.-){%% endif %%}", function(condition, true_content, false_content)
            condition = condition:match("^%s*(.-)%s*$")  -- trim spaces
            local env = setmetatable(ctx, {__index = _G})

            if load("return " .. condition, "condition", "t", env)() then
                return replace_vars(true_content, ctx)
            else
                return replace_vars(false_content, ctx)
            end
        end) or string.gsub(str, "{%% if (.-) %%}(.-){%% endif %%}", function(condition, true_content)
            condition = condition:match("^%s*(.-)%s*$")  -- trim spaces
            local env = setmetatable(ctx, {__index = _G})

            if load("return " .. condition, "condition", "t", env)() then
                return replace_vars(true_content, ctx)
            else
                return ""
            end
        end))
    end

    -- Function to handle variable assignments
    local function handle_set_statements(str, ctx)
        return (string.gsub(str, "{%% set (.-) = (.-) %%}", function(var, expr)
            var = var:match("^%s*(.-)%s*$")  -- trim spaces
            expr = expr:match("^%s*(.-)%s*$")  -- trim spaces
            local env = setmetatable(ctx, {__index = _G})
            ctx[var] = load("return " .. expr, "expression", "t", env)()
            return ""
        end))
    end

    -- Handle loops and conditionals in a specific order
    content = handle_for_in_loops(content, context)
    content = handle_for_loops(content, context)
    content = handle_if_statements(content, context)
    content = replace_vars(content, context)

    return content
end
