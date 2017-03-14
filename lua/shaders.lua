--/////////////
--// SHADERS //
--/////////////

local shaders = {_TYPE='module', _NAME='shaders', _VERSION='0.1'}

-- access to the metatable we set at the end of the file
local matrix_meta = {}

function shaders.load(code, type)
  -- Create the shader
	shaderID = gl.CreateShader(type);

	-- Compile  Shader
	print("Compiling shader : ", code);
	gl.ShaderSource(shaderID, code);
	gl.CompileShader(shaderID);

	-- Check Shader
	result = gl.GetShaderiv(shaderID, gl.COMPILE_STATUS);

  if result == gl.GL_FALSE then
    err = gl.GetShaderInfoLog(shaderID);
    error("shader compile error - " .. err);

    infoLogLength = gl.GetShaderiv(shaderID, gl.INFO_LOG_LENGTH);

    if infoLogLength > 0 then
      err = err .. " " .. gl.GetShaderInfoLog(shaderID);
      error("shader load error - " .. err);
    end
  end

  return shaderID;
end

return shaders;