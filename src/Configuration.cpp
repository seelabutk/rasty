#include "Configuration.h"
#include "ConfigReader.h"
// #include "TransferFunction.h"

#include "rapidjson/document.h"

#include <glob.h>
#include <algorithm>
#include <iostream>
#include <map>

namespace rasty {

Configuration::Configuration(rapidjson::Document& json)
{
    /* the following are required:
     *  - data filename
     *  - data dimensions <- should be required only for raw
     *  - image size/dimensions (ie width and height)
     *  - image filename
     * everything else is optional
     */

    if(!json.HasMember("geoFilename"))
        std::cerr << "Geo filename is required!" << std::endl;
    else {
        std::string fname = json["geoFilename"].GetString();

        // check to see if the filename provided is actually a glob
        glob_t globResult;
        // GLOB_ERR - immediately return on error, don't continue
        // GLOB_BRACE - allow brace expansion
        //              e.g. /path/{one,two} => /path/one /path/two
        // GLOB_NOMAGIC - return the word if there are no metacharacters
        //                in the string, even if there is no match. This
        //                is preferred over GLOB_NOCHECK because it limits
        //                gl_pathv to containing only path(s) without
        //                metacharacters
        // GLOB_TILDE_CHECK - replace ~ or ~username with home directories
        //                    and fail if not found
        unsigned int globError = glob(fname.c_str(),
                GLOB_ERR | GLOB_BRACE | GLOB_TILDE_CHECK | GLOB_NOMAGIC, NULL,
                &globResult);

        if(globError == GLOB_NOSPACE) {
            std::cerr << "ERROR: Ran out of memory when globbing files with";
            std::cerr << "pattern " << fname << std::endl;
        }
        else if(globError == GLOB_ABORTED) {
            std::cerr << "ERROR: Read error when globbing files with";
            std::cerr << "pattern " << fname << std::endl;
        }
        else if(globError == GLOB_NOMATCH) {
            std::cerr << "ERROR: No matches found when globbing files with";
            std::cerr << "pattern " << fname << std::endl;
        }
        else {
            // either there was a glob that returned a single path, or there
            // were no metacharacters to glob with, in which case the path may
            // be valid or invalid
            if(globResult.gl_pathc == 1) {
                this->geoFilename = fname;
            }
            // there was a list of paths successfully globbed
            else {
                for(int i = 0; i < globResult.gl_pathc; i++)
                    this->geoGlobbedFilenames.push_back(globResult.gl_pathv[i]);
            }
        }

        globfree(&globResult);
    }

    if(!json.HasMember("dataFilename"))
        std::cerr << "Data filename is required!" << std::endl;
    else {
        std::string fname = json["dataFilename"].GetString();

        // check to see if the filename provided is actually a glob
        glob_t globResult;
        // GLOB_ERR - immediately return on error, don't continue
        // GLOB_BRACE - allow brace expansion
        //              e.g. /path/{one,two} => /path/one /path/two
        // GLOB_NOMAGIC - return the word if there are no metacharacters
        //                in the string, even if there is no match. This
        //                is preferred over GLOB_NOCHECK because it limits
        //                gl_pathv to containing only path(s) without
        //                metacharacters
        // GLOB_TILDE_CHECK - replace ~ or ~username with home directories
        //                    and fail if not found
        unsigned int globError = glob(fname.c_str(),
                GLOB_ERR | GLOB_BRACE | GLOB_TILDE_CHECK | GLOB_NOMAGIC, NULL,
                &globResult);

        if(globError == GLOB_NOSPACE) {
            std::cerr << "ERROR: Ran out of memory when globbing files with";
            std::cerr << "pattern " << fname << std::endl;
        }
        else if(globError == GLOB_ABORTED) {
            std::cerr << "ERROR: Read error when globbing files with";
            std::cerr << "pattern " << fname << std::endl;
        }
        else if(globError == GLOB_NOMATCH) {
            std::cerr << "ERROR: No matches found when globbing files with";
            std::cerr << "pattern " << fname << std::endl;
        }
        else {
            // either there was a glob that returned a single path, or there
            // were no metacharacters to glob with, in which case the path may
            // be valid or invalid
            if(globResult.gl_pathc == 1) {
                this->dataFilename = fname;
            }
            // there was a list of paths successfully globbed
            else {
                for(int i = 0; i < globResult.gl_pathc; i++)
                    this->dataGlobbedFilenames.push_back(globResult.gl_pathv[i]);
            }
        }

        globfree(&globResult);
    }   


    // if(!json.HasMember("dimensions"))
    //     std::cerr << "Data dimensions are required!" << std::endl;
    // else {
    //     const rapidjson::Value& dataDim = json["dimensions"];
    //     this->dataXDim = dataDim[0].GetInt();
    //     this->dataYDim = dataDim[1].GetInt();
    //     this->dataZDim = dataDim[2].GetInt();
    // }

    if(!json.HasMember("imageSize"))
        std::cerr << "Image dimensions are required!" << std::endl;
    else {
        const rapidjson::Value& imageDim = json["imageSize"];
        this->imageWidth = imageDim[0].GetInt();
        this->imageHeight = imageDim[1].GetInt();
    }

    if(!json.HasMember("outputImageFilename"))
        std::cerr << "Image filename is required!" << std::endl;
    else
        this->imageFilename = json["outputImageFilename"].GetString();

    // background color as list of RGB values, default is black
    if(json.HasMember("backgroundColor")) {
        const rapidjson::Value& bg = json["backgroundColor"];
        for(rapidjson::SizeType i = 0; i < bg.Size(); i++)
            this->bgColor.push_back((unsigned char)bg[i].GetUint());
    }

    // choice of variable for netcdf files
    if(json.HasMember("dataVariable"))
        this->dataVariable = json["dataVariable"].GetString();
    else
        this->dataVariable = "";
    // if no color map is requested, the transfer function will just
    // use a black to white default
    if(json.HasMember("colorMap"))
        this->colorMap = json["colorMap"].GetString();
        // this->selectColorMap(json["colorMap"].GetString());

    // opacity map is a ramp by default, otherwise get a list from the user
    // if(json.HasMember("opacityMap")) {
    //     // opacity map can either be explicit or a named array
    //     if(json["opacityMap"].IsArray()) {
    //         const rapidjson::Value& omap = json["opacityMap"];
    //         for(rapidjson::SizeType i = 0; i < omap.Size(); i++)
    //             this->opacityMap.push_back(omap[i].GetFloat());
    //     }
    //     else {
    //         this->selectOpacityMap(json["opacityMap"].GetString());
    //     }
    // }

    // opacity attenuation >= 1.0 doesn't do anything
    if(json.HasMember("opacityAttenuation"))
        this->opacityAttenuation = json["opacityAttenuation"].GetFloat();
    else
        this->opacityAttenuation = 1.0;

    // samples per pixel
    if(json.HasMember("samplesPerPixel")) {
        unsigned int val = json["samplesPerPixel"].GetUint();
        this->samples = val;
    }
    else
        this->samples = 4;

    // allow a camera position, else use the camera's default of 0,0,0
    if(json.HasMember("cameraPosition")) {
        const rapidjson::Value& cameraPos = json["cameraPosition"];
        this->cameraX = cameraPos[0].GetFloat();
        this->cameraY = cameraPos[1].GetFloat();
        this->cameraZ = cameraPos[2].GetFloat();
    }
    else {
        this->cameraX = 0.0;
        this->cameraY = 0.0;
        this->cameraZ = 0.0;
    }

    // allow a camera up vector, else use the camera's default of 0, 1, 0
    if(json.HasMember("cameraUpVector")) {
        const rapidjson::Value& cameraUpVector = json["cameraUpVector"];
        this->cameraUpX = cameraUpVector[0].GetFloat();
        this->cameraUpY = cameraUpVector[1].GetFloat();
        this->cameraUpZ = cameraUpVector[2].GetFloat();
    }
    else {
        this->cameraUpX = 0.0;
        this->cameraUpY = 1.0;
        this->cameraUpZ = 0.0;
    }

    if(json.HasMember("elevationScale"))
        this->elevationScale = json["elevationScale"].GetFloat();
    else
        this->elevationScale = 1.f / 10000.f;

    if(json.HasMember("heightWidthScale"))
        this->heightWidthScale = json["heightWidthScale"].GetFloat();
    else
        this->heightWidthScale = 1.f;

    // isosurface values for rendering surfaces instead of volume rendering
    // if this is not present, the vector is empty and a volume is rendered
    // if(json.HasMember("isosurfaceValues")) {
    //     // there could be a single value or an array
    //     if(json["isosurfaceValues"].IsArray()) {
    //         const rapidjson::Value& vals = json["isosurfaceValues"];
    //         for(rapidjson::SizeType i = 0; i < vals.Size(); i++)
    //             this->isosurfaceValues.push_back(vals[i].GetFloat());
    //     }
    //     else {
    //         this->isosurfaceValues.push_back(
    //                 json["isosurfaceValues"].GetFloat());
    //     }
    // }
    
    // specularity for surface lighting
    // isosurface renders create a directional light (with direction set
    // to the camera view direction) and this will optionally set the amount
    // of specular reflection, default is 0.1
    if(json.HasMember("specular")) {
        this->specularity =
            std::max(std::min(json["specular"].GetFloat(), 1.f), 0.f);
    }
    else {
        this->specularity = 0.1;
    }
}
/*
void Configuration::selectColorMap(std::string userInput)
{
    // some simple alternatives to color map names are allowed
    if(userInput.compare("blackToWhite") == 0 || 
       userInput.compare("grayscale") == 0 ||
       userInput.compare("greyscale") == 0 ||
       userInput.compare("xray") == 0 ||
       userInput.compare("x-ray") ==0 ) {
       // do nothing, this is the default
    }
    else if(userInput.compare("coolToWarm") == 0 ||
       userInput.compare("cool to warm") == 0) {
        this->colorMap = colormaps["coolToWarm"];
    }
    else if(userInput.compare("spectralReverse") == 0 ||
            userInput.compare("spectral reverse") == 0 ||
            userInput.compare("reverse spectral") == 0 ||
            userInput.compare("reverseSpectral") == 0) {
        this->colorMap = colormaps["spectralReverse"];
    }
    else if(userInput.compare("magma") == 0) {
        this->colorMap = colormaps["magma"];
    }
    else if(userInput.compare("viridis") == 0) {
        this->colorMap = colormaps["viridis"];
    }
    else {
        // does it exist in colormaps
        bool colormap_found = false;
        std::map<std::string, std::vector<float>>::iterator it;
        for(it = colormaps.begin(); it != colormaps.end(); it++)
        {
            if(userInput.compare(it->first) == 0)
            {
                this->colorMap = it->second;
                colormap_found = true;
                break;
            }
        }
        if (colormap_found == false)
        {
            // will default to black to white
            std::cerr << "Unrecognized color map " << userInput << "!" << std::endl;
        }
    }
}
*/
/*
void Configuration::selectOpacityMap(std::string userInput)
{
    if(userInput == "ramp") {
        // do nothing, this is the default
    }
    else if(userInput == "reverseRamp" ||
            userInput == "reverse ramp") {
        this->opacityMap = reverseRamp;
    }
    else if(userInput == "tent" ||
            userInput == "tents" ||
            userInput == "teeth") {
        this->opacityMap = teeth;
    }
    else if(userInput == "exponential") {
        this->opacityMap = exponential;
    }
    else if(userInput == "reverseExponential" ||
            userInput == "reverse exponential" ) {
        this->opacityMap = reverseExponential;
    }
    else if(userInput == "flat") {
        this->opacityMap = flat;
    }
    else {
        // will default to ramp
        std::cerr << "Unrecognized opacity map " << userInput << "!";
        std::cerr << std::endl;
    }
}
*/

CONFSTATE Configuration::getGeoConfigState()
{
    // six possible states for the config/data
    if(this->geoFilename.empty() && this->geoGlobbedFilenames.empty()) {
        // no filename(s) was given
        return ERROR_NODATA;
    }
    if(!this->geoFilename.empty() && !this->geoGlobbedFilenames.empty()) {
        // both a single and globbed set of filenames were given
        return ERROR_MULTISET;
    }

    // if there is not a single filename
    if(this->geoFilename.empty()) {
        // this is either a globbed set with or without a variable
        if(this->geoFilename.empty())
            return MULTI_NOVAR;
        else
            return MULTI_VAR;
    }
    else {
        // otherwise this is a single volume with or without a variable
        if(this->geoFilename.empty())
            return SINGLE_NOVAR;
        else
            return SINGLE_VAR;
    }
}


CONFSTATE Configuration::getDataConfigState()
{
    // six possible states for the config/data
    if(this->dataFilename.empty() && this->dataGlobbedFilenames.empty()) {
        // no filename(s) was given
        return ERROR_NODATA;
    }
    if(!this->dataFilename.empty() && !this->dataGlobbedFilenames.empty()) {
        // both a single and globbed set of filenames were given
        return ERROR_MULTISET;
    }

    // if there is not a single filename
    if(this->dataFilename.empty()) {
        // this is either a globbed set with or without a variable
        if(this->dataFilename.empty())
            return MULTI_NOVAR;
        else
            return MULTI_VAR;
    }
    else {
        // otherwise this is a single volume with or without a variable
        if(this->dataFilename.empty())
            return SINGLE_NOVAR;
        else
            return SINGLE_VAR;
    }
}

}
