#include "MLSMap.hpp"

using namespace envire;

ENVIRONMENT_ITEM_DEF( MLSMap )

Eigen::AlignedBox<double, 2> MLSMap::getExtents() const 
{ 
    // combine the extents of the children of this map
    Eigen::AlignedBox<double, 2> extents;

    std::list<const Layer*> children = env->getChildren( this );
    for( std::list<const Layer*>::const_iterator it = children.begin(); it != children.end(); it++ )
    {
	const envire::MLSGrid *grid = dynamic_cast<const envire::MLSGrid*>( *it );
	// currently this assumes that child grids are not rotated
	// TODO handle rotated child grids here
	Eigen::Affine3d g2m = grid->getFrameNode()->relativeTransform( this->getFrameNode() );
	extents.extend( grid->getExtents().translate( g2m.translation().head<2>() ) );
    }

    return extents;
}

MLSMap::MLSMap()
{
}

MLSMap::MLSMap(const MLSMap& other)
    : grids( other.grids ), active( other.active )
{
}

MLSMap& MLSMap::operator=(const MLSMap& other)
{
    if( this != &other )
    {
	Map<2>::operator=( other );

	grids = other.grids;
	active = other.active;

	if( isAttached() )
	{
	    std::list<Layer*> children = env->getChildren( this );
	    for( std::list<Layer*>::iterator it = children.begin(); it != children.end(); it++ )
	    {
		env->removeChild( this, *it );
	    }

	    typedef std::vector<MLSGrid::Ptr>::iterator iterator;
	    for( iterator it = grids.begin(); it != grids.end(); it++ )
	    {
		env->addChild( this, (*it).get() );
	    }
	}
    }

    return *this;
}

inline bool getPatch( MLSGrid* grid, const Transform& C_m2g, const Point& p, MLSGrid::SurfacePatch& patch, double sigma_threshold )
{
    MLSGrid::Position pos;
    if( grid->toGrid((C_m2g * p).head<2>(), pos) )
    {
	// offset the z-coordinate which is given in map to grid 
	MLSGrid::SurfacePatch probe( patch );
	probe.mean += C_m2g.translation().z();

	MLSGrid::SurfacePatch* res = 
	    grid->get( pos, probe, sigma_threshold );

	if( res )
	{
	    patch = *res;
	    // offset the z-coordinate, so that it is expressed in terms
	    // of the map and not the grid
	    patch.mean -= C_m2g.translation().z();
	    return true;
	}
    }
    return false;
}

bool MLSMap::getPatch( const Point& p, SurfacePatch& patch, double sigma_threshold )
{
    // see if we can use the cache. This will reduce the amount of transform
    // calculations
    if( cache.grid )
	if( ::getPatch( cache.grid, cache.trans, p, patch, sigma_threshold ) )
	    return true;

    // go backwards in the list, and try to find the point in any of the grids
    for( std::vector<MLSGrid::Ptr>::reverse_iterator it = grids.rbegin(); it != grids.rend(); it++ )
    {
	MLSGrid *grid( it->get() );
	Transform C_m2g = env->relativeTransform( getFrameNode(), grid->getFrameNode() );
	if( ::getPatch( grid, C_m2g, p, patch, sigma_threshold ) )
	{
	    cache.grid = it->get();
	    cache.trans = C_m2g;
	    return true;
	}
    }
    return false;
}

void MLSMap::addGrid( MLSGrid::Ptr grid )
{
    env->addChild( this, grid.get() );

    grids.push_back( grid );
    active = grid;
}

void MLSMap::selectActiveGrid( const FrameNode* fn, double threshold, bool aligned  )
{
    // go through the grids, and store the ones where the center is within threshold
    MLSGrid* best_grid = NULL;
    double best_dist = threshold; 

    for( size_t i=0; i<grids.size(); ++i )
    {
	Transform t = fn->relativeTransform( grids[i]->getFrameNode() );
	double dist = std::max( fabs(t.translation().x()), fabs(t.translation().y() ) );
	if( dist < best_dist )
	{
	    best_grid = grids[i].get();
	    best_dist = dist;
	}
    }

    if( best_grid )
    {
	active = best_grid;
    }
    else
    {
	Transform t = fn->relativeTransform( getActiveGrid()->getFrameNode() );
	createGrid( t, true, true );
    }
}

void MLSMap::createGrid( const Transform& trans, bool relative, bool aligned )
{
    // don't do anything if there is no template 
    if( !active )
	return;

    Transform t = trans;
    if( aligned )
    {
	// remove rotation
	t.linear().setIdentity();
	// align to grid size
	t.translation().x() = floor(t.translation().x() / active->getScaleX()) * active->getScaleX();
	t.translation().y() = floor(t.translation().y() / active->getScaleY()) * active->getScaleY();
    }

    MLSGrid* grid_clone = active->cloneShallow(); 
    FrameNode* fn = new FrameNode( t );
    fn->setUniqueId( getUniqueIdPrefix() + "/" );
    if( relative )
	env->addChild( active->getFrameNode(), fn );
    else
	env->addChild( getFrameNode(), fn );
    env->setFrameNode( grid_clone, fn );

    addGrid( grid_clone );
}

MLSMap* MLSMap::cloneDeep()
{
    MLSMap* res = clone();
    res->setUniqueId( getUniqueIdPrefix() + "/" );
    // copy the layer structure as well
    if( env )
    {
	// create a copy of the currently active map
	// and reference the others
	MLSGrid* active_clone = active->clone();
	active_clone->setUniqueId( getUniqueIdPrefix() + "/" );
	std::vector<MLSGrid::Ptr>::iterator it = 
	    std::find( res->grids.begin(), res->grids.end(), active );

	assert( it != res->grids.end() );
	*it = active_clone;
	res->active = active_clone;

	env->setFrameNode( active_clone, active->getFrameNode() );
	
	env->attachItem( res );

	typedef std::vector<MLSGrid::Ptr>::iterator iterator;
	for( iterator it = res->grids.begin(); it != res->grids.end(); it++ )
	{
	    env->addChild( res, (*it).get() );
	}
    }
    return res;
}

