/*
 Copyright (c) 2013, The Cinder Project, All rights reserved.

 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "cinder/DataSource.h"
#include "cinder/DataTarget.h"
#include "cinder/GeomIo.h"

#include <boost/tuple/tuple_comparison.hpp>
#include <map>

namespace cinder {

/** \brief Loads Alias|Wavefront .OBJ file format
 *
 * Example usage:
 * \code
 * cinder::gl::BatchRef myCubeRef;
 * ObjLoader loader( loadFile( "myPath/cube.obj" ) );
 * myCubeRef = gl::Batch::create( loader, gl::getStockShader( gl::ShaderDef().color() ) );
 * myCubeRef->draw();
 * \endcode
**/

class ObjLoader : public geom::Source {
  public:
	/**Constructs and does the parsing of the file
	 * \param loadNormals if false texture coordinates will be skipped, which can provide a faster load time
	 * \param loadTexCoords if false normasls will be skipped, which can provide a faster load time
	**/
	ObjLoader( std::shared_ptr<IStreamCinder> stream, bool loadNormals = true, bool loadTexCoords = true );
	/**Constructs and does the parsing of the file
	 * \param loadNormals if false texture coordinates will be skipped, which can provide a faster load time
	 * \param loadTexCoords if false normasls will be skipped, which can provide a faster load time
	**/
	ObjLoader( DataSourceRef dataSource, bool loadNormals = true, bool loadTexCoords = true );
	/**Constructs and does the parsing of the file
	 * \param loadNormals if false texture coordinates will be skipped, which can provide a faster load time
	 * \param loadTexCoords if false normasls will be skipped, which can provide a faster load time
     **/
	ObjLoader( DataSourceRef dataSource, DataSourceRef materialSource, bool loadNormals = true, bool loadTexCoords = true );

	/**Loads a specific group index from the file**/
	ObjLoader&	groupIndex( size_t groupIndex );
	/**Loads a specific group name from the file**/
	ObjLoader&	groupName( const std::string &groupName );

	struct Material {
        Material() {
            Ka[0] = Ka[1] = Ka[2] = 0;
            Kd[0] = Kd[1] = Kd[2] = 1;
        }

        Material( const Material& rhs ) {
            mName = rhs.mName;
            Ka[0] = rhs.Ka[0];
            Ka[1] = rhs.Ka[1];
            Ka[2] = rhs.Ka[2];
            Kd[0] = rhs.Kd[0];
            Kd[1] = rhs.Kd[1];
            Kd[2] = rhs.Kd[2];
        }

        std::string mName;
        float		Ka[3];
        float		Kd[3];
    };
    
	struct Face {
		int						mNumVertices;
		std::vector<int32_t>	mVertexIndices;
		std::vector<int32_t>	mTexCoordIndices;
		std::vector<int32_t>	mNormalIndices;
		const Material*			mMaterial;
	};

	struct Group {
		std::string				mName;
		int32_t					mBaseVertexOffset, mBaseTexCoordOffset, mBaseNormalOffset;
		std::vector<Face>		mFaces;
		bool					mHasTexCoords;
		bool					mHasNormals;
	};

	//! Returns the total number of groups.
	size_t		getNumGroups() const { return mGroups.size(); }
	
	//! Returns a vector<> of the Groups in the OBJ.
	const std::vector<Group>&		getGroups() const { return mGroups; }

	size_t			getNumVertices() const override { load(); return mOutputVertices.size(); }
	size_t			getNumIndices() const override { load(); return mOutputIndices.size(); }
	geom::Primitive	getPrimitive() const override { return geom::Primitive::TRIANGLES; }
	uint8_t			getAttribDims( geom::Attrib attr ) const override;
	geom::AttribSet	getAvailableAttribs() const override;
	void			loadInto( geom::Target *target, const geom::AttribSet &requestedAttribs ) const override;

  private:
	typedef boost::tuple<int,int> VertexPair;
	typedef boost::tuple<int,int,int> VertexTriple;

	void	parse( bool loadNormals, bool loadTexCoords );
 	void	parseFace( Group *group, const Material *material, const std::string &s, bool loadNormals, bool loadTexCoords );
    void    parseMaterial( std::shared_ptr<IStreamCinder> material );

	void	loadGroupNormalsTextures( const Group &group, std::map<boost::tuple<int,int,int>,int> &uniqueVerts ) const;
	void	loadGroupNormals( const Group &group, std::map<boost::tuple<int,int>,int> &uniqueVerts ) const;
	void	loadGroupTextures( const Group &group, std::map<boost::tuple<int,int>,int> &uniqueVerts ) const;
	void	loadGroup( const Group &group, std::map<int,int> &uniqueVerts ) const;

	void	load() const;

	std::shared_ptr<IStreamCinder>	mStream;

	std::vector<vec3>			    mInternalVertices, mInternalNormals;
	std::vector<vec2>			    mInternalTexCoords;
	std::vector<Colorf>				mInternalColors;

	mutable bool					mOutputCached;
	mutable std::vector<vec3>		mOutputVertices, mOutputNormals;
	mutable std::vector<vec2>		mOutputTexCoords;
	mutable std::vector<Colorf>		mOutputColors;
	mutable std::vector<uint32_t>	mOutputIndices;

	size_t							mGroupIndex;
	std::vector<Group>				mGroups;
	std::map<std::string, Material>	mMaterials;


};

//! Writes a new OBJ file to \a dataTarget.
void	objWrite( DataTargetRef dataTarget, const geom::Source &source, bool includeNormals = true, bool includeTexCoords = true );	
inline void	objWrite( DataTargetRef dataTarget, const geom::SourceRef &source, bool includeNormals = true, bool includeTexCoords = true )
{
	objWrite( dataTarget, *source, includeNormals, includeTexCoords );
}

} // namespace cinder
