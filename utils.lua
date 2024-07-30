isnumber = function( n )
    return type( n ) == "number"
end

isstring = function( s )
    return type( s ) == "string"
end

isboolean = function( b )
    return type( b ) == "boolean"
end

istable = function( t )
    return type( t ) == "table"
end




dofile("table.lua")

function printTable( t, indent, done )
	local Msg = Msg

	done = done or {}
	indent = indent or 0
	local keys = table.GetKeys( t )

	table.sort( keys, function( a, b )
		if ( isnumber( a ) and isnumber( b ) ) then return a < b end
		return tostring( a ) < tostring( b )
	end )

	done[ t ] = true

	for i = 1, #keys do
		local key = keys[ i ]
		local value = t[ key ]
		key = ( type( key ) == "string" ) and "[\"" .. key .. "\"]" or "[" .. tostring( key ) .. "]"
		print( string.rep( "\t", indent ) )

		if  ( istable( value ) and not done[ value ] ) then

			done[ value ] = true
			print( key, ":\n" )
			printTable ( value, indent + 2, done )
			done[ value ] = nil

		else

			print( key, "\t=\t", value, "\n" )

		end

	end

end