matrix = dofile("lua/matrix.lua");
shaders = dofile("lua/shaders.lua");

function loadTextFromFile(filePath)
	local file = io.open(filePath);
	local result = "";

	for line in file:lines() do
			result = result .. line;
	end

	return result;
end

function loadShaders(vertexShaderPath, fragmentShaderPath)
  vertexShaderCode = loadTextFromFile(vertexShaderPath);

  vertexShaderID = shaders.load(vertexShaderCode, gl.VERTEX_SHADER);

  fragmentShaderCode = loadTextFromFile(fragmentShaderPath);

  fragmentShaderID = shaders.load(fragmentShaderCode, gl.FRAGMENT_SHADER);

  -- Link the program
  print("Linking program\n");

  programID = gl.CreateProgram();

	print("Attaching shader " .. vertexShaderID);
  gl.AttachShader(programID, vertexShaderID);

	print("Attaching shader " .. fragmentShaderID);
  gl.AttachShader(programID, fragmentShaderID);

	print("Link Program " .. programID);
  gl.LinkProgram(programID);


  -- Check the program
  result = gl.GetProgramiv(programID, gl.LINK_STATUS);

	if result == gl.GL_FALSE then
	  infoLogLength = gl.GetProgramiv(programID, gl.INFO_LOG_LENGTH);

	  if infoLogLength > 0 then
	    err = gl.GetProgramInfoLog(programID);
	    error("program load error - " .. err);
	  end
	end

  gl.DetachShader(programID, vertexShaderID);
  gl.DetachShader(programID, fragmentShaderID);

  gl.DeleteShader(vertexShaderID);
  gl.DeleteShader(fragmentShaderID);

  return programID;
end

function update()

end

function draw()

  local verts = 
  {
    -1.0,-1.0,-1.0, 
    -1.0,-1.0, 1.0,
    -1.0, 1.0, 1.0, 
    1.0, 1.0,-1.0, 
    -1.0,-1.0,-1.0,
    -1.0, 1.0,-1.0, 
    1.0,-1.0, 1.0,
    -1.0,-1.0,-1.0,
    1.0,-1.0,-1.0,
    1.0, 1.0,-1.0,
    1.0,-1.0,-1.0,
    -1.0,-1.0,-1.0,
    -1.0,-1.0,-1.0,
    -1.0, 1.0, 1.0,
    -1.0, 1.0,-1.0,
    1.0,-1.0, 1.0,
    -1.0,-1.0, 1.0,
    -1.0,-1.0,-1.0,
    -1.0, 1.0, 1.0,
    -1.0,-1.0, 1.0,
    1.0,-1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0,-1.0,-1.0,
    1.0, 1.0,-1.0,
    1.0,-1.0,-1.0,
    1.0, 1.0, 1.0,
    1.0,-1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0,-1.0,
    -1.0, 1.0,-1.0,
    1.0, 1.0, 1.0,
    -1.0, 1.0,-1.0,
    -1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    -1.0, 1.0, 1.0,
    1.0,-1.0, 1.0
};

  gl.ClearColor( 0, 0, 1, 0 );
  gl.Clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

  local vertexArrayID = gl.GenVertexArray();

  gl.BindVertexArray(vertexArrayID);

  -- This will identify our vertex buffer
  -- Generate 1 buffer, put the resulting identifier in vertexbuffer
  local vertexbuffer = gl.GenBuffer();
  -- The following commands will talk about our 'vertexbuffer' buffer
  gl.BindBuffer(gl.ARRAY_BUFFER, vertexbuffer);
  -- Give our vertices to OpenGL.
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
  gl.DrawArrays(gl.TRIANGLES, 0, #verts/3); -- Starting from vertex 0; 3 vertices total -> 1 triangle
  gl.DisableVertexAttribArray(0);
end

function awake()
  mtx = matrix.perspective(math.rad(45),640/480,0.1,100);

  -- Create The Native Window
  CreateWindow();

  programID = loadShaders("lua/shaders/vertex.shader", "lua/shaders/fragment.shader");

  gl.UseProgram(programID);
end