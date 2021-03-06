#include <Eigen/Geometry>
#include "icp/icp.hpp"
#include "icp/ransac.hpp"

#include "envire/Core.hpp"
#include "envire/maps/TriMesh.hpp"

#include "boost/scoped_ptr.hpp"

#define BOOST_TEST_MODULE ICPTest 
#include <boost/test/included/unit_test.hpp>

using namespace envire;
using namespace std;

struct ICPTest
{
    ICPTest()
	: env( new Environment() )
    {
    }

    void setHalfCube(envire::Pointcloud *mesh)
    {
	std::vector<Eigen::Vector3d>& points(mesh->vertices);
	std::vector<envire::TriMesh::vertex_attr>& attr(mesh->getVertexData<envire::TriMesh::vertex_attr>(envire::TriMesh::VERTEX_ATTRIBUTES));
	std::vector<Eigen::Vector3d>& normal(mesh->getVertexData<Eigen::Vector3d>(envire::TriMesh::VERTEX_NORMAL));

	// generate a pointcloud with vertices on 3 adjecent walls of a cube
	for(int i=0;i<=10;i++)
	{
	    for(int j=0;j<=10;j++)
	    {
		bool edge = (i == 10 || j == 10);

		double x = i*.1;
		double y = j*.1;

		points.push_back( Eigen::Vector3d( x, y, 0 ) );
		points.push_back( Eigen::Vector3d( 0, x, y ) );
		points.push_back( Eigen::Vector3d( x, 0, y ) );
		attr.push_back( edge << TriMesh::SCAN_EDGE );
		attr.push_back( edge << TriMesh::SCAN_EDGE );
		attr.push_back( edge << TriMesh::SCAN_EDGE );
		normal.push_back( Eigen::Vector3d( 0, 0, 1 ) );
		normal.push_back( Eigen::Vector3d( 1, 0, 0 ) );
		normal.push_back( Eigen::Vector3d( 0, 1, 0 ) );
	    }
	}
    }

    void setSineWave(envire::Pointcloud *mesh)
    {
	std::vector<Eigen::Vector3d>& points(mesh->vertices);
	std::vector<envire::TriMesh::vertex_attr>& attr(mesh->getVertexData<envire::TriMesh::vertex_attr>(envire::TriMesh::VERTEX_ATTRIBUTES));
	std::vector<Eigen::Vector3d>& normal(mesh->getVertexData<Eigen::Vector3d>(envire::TriMesh::VERTEX_NORMAL));

	const int xmin = -10, xmax = 10;
	const int ymin = -10, ymax = 10;
	double zscale = .2;
	double yscale = 10.0;

	for(int i=xmin;i<=xmax;i++)
	{
	    for(int j=ymin;j<=ymax;j++)
	    {
		bool edge = (i == xmin || j == ymin || i == xmax || j == ymax );

		double x = i*.1;
		double y = j*.1;
		double d = sqrt( x*x + y*y ); 
		double z = zscale*cos( yscale * d );

		double zd = -zscale*yscale*sin( yscale * d );
		double nx = sqrt(1.0/(1.0+zd*zd));
		double ny = zd * nx;

    
		Eigen::Vector3d norm( 
			-ny,
			0,
			nx );

		norm = Eigen::AngleAxisd(atan2(y, x), Eigen::Vector3d::UnitZ()) * norm;

		points.push_back( Eigen::Vector3d( x, y, z ) );
		attr.push_back( edge << TriMesh::SCAN_EDGE );
		normal.push_back(norm);
	    }
	}
    }

    enum test_case
    {
	sine, 
	box
    };

    void setTestEnvironment( test_case tc, const Eigen::Affine3d& a_trans, const Eigen::Affine3d& b_trans )
    {
	FrameNode *fm1 = new FrameNode();
	FrameNode *fm2 = new FrameNode();
	env->addChild( env->getRootNode(), fm1 );
	env->addChild( env->getRootNode(), fm2 );

	mesh = new Pointcloud();
	env->attachItem( mesh );

	mesh2 = new Pointcloud();
	env->attachItem( mesh2 );

	switch( tc )
	{
	    case sine:
		setSineWave( mesh ); setSineWave( mesh2 ); break;
	    case box:
		setHalfCube( mesh ); setHalfCube( mesh2 ); break;
	}

	mesh->setFrameNode( fm1 );
	mesh2->setFrameNode( fm2 );

	fm1->setTransform( a_trans );
	fm2->setTransform( b_trans );

    }

    boost::scoped_ptr<Environment> env;
    Pointcloud *mesh, *mesh2;
};

/*
BOOST_AUTO_TEST_CASE( icp_test1 )
{
    ICPTest test;
    test.setTestEnvironment( ICPTest::sine, 
	    Eigen::Affine3d( Eigen::Affine3d::Identity() ),
	    Eigen::Translation3d( 0.0,0,0 )
	    * Eigen::AngleAxisd( 0, Eigen::Vector3d::UnitX()) );

    envire::icp::TrimmedKDEAN icp;
    icp.addToModel( envire::icp::PointcloudEdgeAndNormalAdapter( test.mesh, 1.0 ) );
    icp.align( envire::icp::PointcloudEdgeAndNormalAdapter( test.mesh2, 1.0 ), 10, 1e-4, 1e-5 );
} 
*/

BOOST_AUTO_TEST_CASE( icp_test2 )
{
    ICPTest test;
    test.setTestEnvironment( ICPTest::sine, 
	    Eigen::Affine3d( Eigen::Affine3d::Identity() ),
	    Eigen::Translation3d( 0,0,5.0 )
	    * Eigen::AngleAxisd( 0.1, Eigen::Vector3d::UnitX()) );

    envire::icp::TrimmedKD icp;
    icp.addToModel( envire::icp::PointcloudEdgeAndNormalAdapter( test.mesh, 1.0 ) );

    double alpha = 0.4, beta = 1.0, eps = 0.05;
    icp.align( envire::icp::PointcloudEdgeAndNormalAdapter( test.mesh2, 1.0 ), 10, 1e-4, 1e-5, alpha, beta, eps );

    test.env.get()->serialize( "/tmp/test" );
} 

using namespace envire::ransac;

BOOST_AUTO_TEST_CASE( ransac_test )
{
    std::cout << "RANSAC test" << std::endl;

    std::vector<Eigen::Vector3d> x, p;
    Eigen::Affine3d model = Eigen::Translation3d( 0, 0, 0.5 ) * 
	Eigen::AngleAxisd( M_PI/8.0, Eigen::Vector3d::UnitX() );

    const double outlierProb = 0.5;

    size_t outliers = 0;
    for( int i = 0; i < 50; i++ )
    {
	Eigen::Vector3d v1 = Eigen::Vector3d::Random() * 10.0;
	Eigen::Vector3d v2 = model * v1;

	if( rand() % 100 < (outlierProb * 100) )
	{
	    outliers++;
	    v2 += Eigen::Vector3d::Random() * 10.0;
	}

	x.push_back( v1 );
	p.push_back( v2 );
    }
    std::cout << "outliers: " << outliers << std::endl;

    Eigen::Affine3d best_model;
    vector_size_t best_inliers;
    const double DIST_THRESHOLD = 0.2;

    //ModelSearch search;
    FitTransform fit( x, p );
    ransacSingleModel( fit, 3, DIST_THRESHOLD, best_model, best_inliers, 1000 );

    std::cout << best_inliers.size() << std::endl;
}
