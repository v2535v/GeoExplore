/**
 * @file    Options.cpp
 * @author  Marvin Smith
 * @date    8/2/2014
*/
#include "Options.hpp"

// C++ Standard Libraries
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

// GeoExplore Libraries
#include "../../thirdparty/pugixml/pugixml.hpp"


namespace bf = boost::filesystem;

/**
 * Options
*/
Options::Options()
  : m_configuration_file_provided(false)
{

}

/**
 * Finalize the configuration
*/
void Options::Finalize(){

    // Load the configuration file if provided
    if( m_configuration_file_provided == true ){
        Load_Configuration_File();
    }

}


/**
 * Load the configuration file
*/
void Options::Load_Configuration_File()
{

    // make sure the config file exists
    if( bf::exists( m_configuration_file ) == false ){
        std::cerr << "error: Configuration File (" << m_configuration_file << ") does not exist." << std::endl;
        std::exit(-1);
    }
    
    // Open the file
    pugi::xml_document doc;

    // Parse the document
    pugi::xml_parse_result result = doc.load_file(m_configuration_file.c_str());

    // Look for root node
    pugi::xml_node root = doc.child("geo-utility");

    // Iterate over nodes
    for (pugi::xml_node_iterator cit = root.begin(); cit != root.end(); cit++ ){

        // Parse the render-configuration
        if( std::string(cit->name()) == "render-configuration" ){
            Parse_Render_Configuration( *cit );
        }
    }

}

/**
 * Parse the render-configuration
*/
void Options::Parse_Render_Configuration( pugi::xml_node& render_config_node )
{
    
    // Temp variables
    double tempDouble;
    int tempInt;
    char tempChar;
    std::string tempStr;

    // Get the image information
    pugi::xml_node img_node = render_config_node.child("image");
    int cols = img_node.attribute("cols").as_int();
    int rows = img_node.attribute("rows").as_int();
    m_image_size = GEO::A_Size_i( cols, rows);

    // Get the bound information
    pugi::xml_node bound_node = render_config_node.child("bounds"); 
    
    // Min Node
    pugi::xml_node min_node = bound_node.child("min");
    pugi::xml_node max_node = bound_node.child("max");

    // Grid Zone
    m_min_image_bound.zone() = min_node.attribute("zone").as_int();
    
    // Hemi
    tempStr = min_node.attribute("hemi").as_string();
    if( tempStr == "N" || tempStr == "n" ){
        m_min_image_bound.Is_Northern_Hemisphere() = true;
    }
    else if( tempStr == "S" || tempStr == "s" ){
        m_min_image_bound.Is_Northern_Hemisphere() = false;
    }
    else{
        throw std::runtime_error("unknown hemisphere.");
    }

    // Easting
    m_min_image_bound.easting() = min_node.attribute("easting").as_double();

    // Northing
    m_min_image_bound.northing() = min_node.attribute("northing").as_double();

    // Grid Zone
    m_max_image_bound.zone() = max_node.attribute("zone").as_int();

    // Hemi
     tempStr = max_node.attribute("hemi").as_string();
    if( tempStr == "N" || tempStr == "n" ){
        m_max_image_bound.Is_Northern_Hemisphere() = true;
    }
    else if( tempStr == "S" || tempStr == "s" ){
        m_max_image_bound.Is_Northern_Hemisphere() = false;
    }
    else{
        throw std::runtime_error("unknown hemisphere.");
    }

    // Easting
    m_max_image_bound.easting() = min_node.attribute("easting").as_double();

    // Northing
    m_max_image_bound.northing() = max_node.attribute("northing").as_double();


}

