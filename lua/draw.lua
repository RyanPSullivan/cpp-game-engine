matrix = dofile("lua/matrix.lua");


function loadShader(code, type)
  -- Create the shader
	shaderID = gl.CreateShader(type);

	-- Compile  Shader
	print("Compiling shader : ", code);
	gl.ShaderSource(shaderID, code);
	gl.CompileShader(shaderID);

	-- Check Shader
	result = gl.GetShaderiv(shaderID, gl.COMPILE_STATUS);
	InfoLogLength = gl.GetShaderiv(shaderID, gl.INFO_LOG_LENGTH);

	if InfoLogLength > 0 then
    error = gl.GetShaderInfoLog(shaderID);
    print("shader load error - ", error);
	end

  return shaderID;
end

function loadShaders()

  vertexShaderCode = "\n \
    attribute vec3 vertexPosition_modelspace;\n \
    void main(){gl_Position.xyz = vertexPosition_modelspace;gl_Position.w = 1.0;}";

  vertexShaderID = loadShader(vertexShaderCode, gl.VERTEX_SHADER);

  fragmentShaderCode = "void main(){gl_FragColor = vec4(0,1,0,0);}";

  fragmentShaderID = loadShader(fragmentShaderCode, gl.FRAGMENT_SHADER);

  -- Link the program
  print("Linking program\n");
  programID = gl.CreateProgram();
  gl.AttachShader(programID, vertexShaderID);
  gl.AttachShader(programID, fragmentShaderID);
  gl.LinkProgram(programID);

  -- Check the program
  result = gl.GetProgramiv(programID, gl.LINK_STATUS);
  infoLogLength = gl.GetProgramiv(programID, gl.INFO_LOG_LENGTH);
  if infoLogLength > 0 then
    error = gl.GetProgramInfoLog(programID);
    print("program load error - ", error);
  end

  gl.DetachShader(programID, vertexShaderID);
  gl.DetachShader(programID, fragmentShaderID);

  gl.DeleteShader(vertexShaderID);
  gl.DeleteShader(fragmentShaderID);

  return programID;
end

function drawTriangle()
  --DRAW BELOW
  local verts = {-1.0, -1.0, 0.0,
  1.0, -1.0, 0.0,
  0.0,  1.0, 0.0};

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
  gl.DrawArrays(gl.TRIANGLES, 0, 3); -- Starting from vertex 0; 3 vertices total -> 1 triangle
  gl.DisableVertexAttribArray(0);
end

mtx = matrix {{1,2},{3,4}}

-- Create The Native Window
CreateWindow();

programID = loadShaders();

gl.UseProgram(programID);

drawTriangle();