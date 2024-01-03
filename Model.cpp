#include "Model.h"

Model::Model(string const& path, bool gamma) : gammaCorrection(gamma)
{
	loadModel(path);
}

void Model::loadModel(string const& path)
{
}

