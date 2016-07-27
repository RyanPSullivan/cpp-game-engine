for k,v in pairs(_G) do
    print("Global key", k, "value", v)
end

print("")

for k,v in pairs(table) do
    print("Global key", k, "value", v)
end

io.write("Hello The World\n");

CreateWindow();

local verts = {-1.0, -1.0, 0.0,
1.0, -1.0, 0.0,
0.0,  1.0, 0.0};

gl.ClearColor( 1, 0, 0, 0 );
gl.Clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

local vertexArrayID = gl.GenVertexArray();

gl.BindVertexArray(vertexArrayID);

-- This will identify our vertex buffer
-- Generate 1 buffer, put the resulting identifier in vertexbuffer
local vertexbuffer = gl.GenBuffer();
-- The following commands will talk about our 'vertexbuffer' buffer
gl.BindBuffer(gl.ARRAY_BUFFER, vertexbuffer);
-- Give our vertices to OpenGL.
for k,v in pairs(verts) do
    print("Global key", k, "value", v)
end
gl.BufferData(gl.ARRAY_BUFFER, verts, gl.STATIC_DRAW);

-- 1st attribute buffer : vertices
gl.EnableVertexAttribArray(0);
gl.BindBuffer(gl.ARRAY_BUFFER, vertexbuffer);
gl.VertexAttribPointer(
   0,
   3,
   gl.FLOAT,
   gl.FALSE,
   0
);

-- Draw the triangle !
gl.DrawArrays(gl.TRIANGLES, 0, 3); -- Starting from vertex 0; 3 vertices total -> 1 triangle
gl.DisableVertexAttribArray(0);
print("WHAT", vertexArrayID);
