/**
 * @file    CoordinateBase.hpp
 * @author  Marvin Smith
 * @date    4/12/2014
 */
#ifndef __SRC_COORDINATE_COORDINATEBASE_HPP__
#define __SRC_COORDINATE_COORDINATEBASE_HPP__

/// GeoExplore Libraries
#include <GeoExplore/core/Enumerations.hpp>

// C++ Standard Libraries
#include <memory>

namespace GEO{

/**
 * @class CoordinateBase
 *
 * This is just a simple Cartesian Coordinate for abstracting more
 * complicated designs from.
 */
template<typename DATATYPE>
class CoordinateBase{


    public:
        
        /// Typedef 
        typedef DATATYPE datatype;

        /// Pointer type
        typedef std::shared_ptr<CoordinateBase<DATATYPE> > ptr_t;
    
        
        /**
         * @brief Constructor
         */
        CoordinateBase() : m_altitude(0), m_datum(Datum::WGS84){};

        /**
         * @brief Constructor Given only the Datum and altiutude
         */
        CoordinateBase( const DATATYPE& altitude, 
                        const Datum& datum ) : 
                            m_altitude(altitude), m_datum(datum)
        {
        }
        
        /**
         * Get the altitude
         */
        inline DATATYPE altitude()const{ 
            return m_altitude; 
        }

        /**
         * Set the altitude
         */
        inline DATATYPE& altitude(){ 
            return m_altitude; 
        }
        
        /**
         * Get the coordinate system
         */
        inline Datum datum()const{
            return m_datum; 
        }

        /**
         * Set the coordinate system
         */
        inline Datum& datum(){ 
            return m_datum; 
        }

        
        /**
         * Clone the object
         */
        CoordinateBase<DATATYPE>::ptr_t clone()const{ 
            return CoordinateBase<DATATYPE>::ptr_t( new CoordinateBase<DATATYPE>(m_altitude, m_datum)); 
        }


        /*
         * Get the type
         */
        virtual CoordinateType type(){ 
            return CoordinateType::Base; 
        }

    protected:
        
        /// Coordinate Information
        DATATYPE m_altitude;
        
        /// Projection Type
        Datum m_datum;
        

}; /// End of CoordinateBase class

/// Common Typdefs
typedef CoordinateBase<double> CoordinateBaseDouble;
typedef CoordinateBase<double> CoordinateBase_d;


} /// End of GEO Namespace

#endif
