#version 400 core

in vec3 vColour;			// Note: colour is smoothly interpolated (default)
out vec4 vOutputColour;

in float fIntensity;

void main()
{	

	if(fIntensity > 0.75f){
		vOutputColour = vec4(1., 1.0, 1.0, 1.0);
	}

	else if(fIntensity>0.5f){
	vOutputColour = vec4(0.6, 0.6, 0.6, 0.6f);
	}

	else{
	vOutputColour = vec4(0.3, 0.3 ,0.3 , 0.3f);
	}
}
