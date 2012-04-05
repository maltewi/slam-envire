#include "GridBase.hpp"
#include "Grid.hpp"

using namespace envire;

const std::string GridBase::className = "envire::GridBase";

GridBase::GridBase(std::string const& id)
    : Map<2>(id)
    , cellSizeX(0), cellSizeY(0), scalex(0), scaley(0), offsetx(0), offsety(0) {}

GridBase::GridBase(size_t cellSizeX, size_t cellSizeY,
        double scalex, double scaley, double offsetx, double offsety,
        std::string const& id)
    : Map<2>(id)
    , cellSizeX(cellSizeX), cellSizeY(cellSizeY)
    , scalex(scalex), scaley(scaley)
    , offsetx(offsetx), offsety(offsety)
{
}

GridBase::~GridBase()
{
}

void GridBase::serialize(Serialization& so)
{
    CartesianMap::serialize(so);

    so.write("cellSizeX", cellSizeX );
    so.write("cellSizeY", cellSizeY );
    so.write("scalex", scalex );
    so.write("scaley", scaley );
    so.write("offsetx", offsetx );
    so.write("offsety", offsety );

    // For backward compatibility with older versions of envire
    so.write("width", cellSizeX );
    so.write("height", cellSizeY );
}

void GridBase::unserialize(Serialization& so)
{
    CartesianMap::unserialize(so);
    
    if (!so.read("cellSizeX", cellSizeX ))
        so.read("width", cellSizeX);
    if (!so.read("cellSizeY", cellSizeY ))
        so.read("height", cellSizeY);

    so.read("scalex", scalex );
    so.read("scaley", scaley );
    so.read("offsetx", offsetx );
    so.read("offsety", offsety );
}

bool GridBase::toGrid( Eigen::Vector3d const& point,
        size_t& xi, size_t& yi, FrameNode const* frame) const
{
    if (frame)
    {
        Eigen::Vector3d transformed = toMap(point, *frame);
        return toGrid(transformed.x(), transformed.y(), xi, yi);
    }
    else
    {
        return toGrid(point.x(), point.y(), xi, yi);
    }
}

bool GridBase::toGrid( double x, double y, size_t& xi, size_t& yi) const
{
    size_t am = floor((x-offsetx)/scalex);
    size_t an = floor((y-offsety)/scaley);
    if( 0 <= am && am < cellSizeX && 0 <= an && an < cellSizeY )
    {
	xi = am;
	yi = an;
	return true;
    }
    else {
	return false;
    }
}

Eigen::Vector3d GridBase::fromGrid(size_t xi, size_t yi, FrameNode const* frame) const
{
    double map_x, map_y;
    fromGrid(xi, yi, map_x, map_y);
    if (frame)
    {
        return fromMap(Eigen::Vector3d(map_x, map_y, 0), *frame);
    }
    else
    {
        return Eigen::Vector3d(map_x, map_y, 0);
    }
}

void GridBase::fromGrid( size_t xi, size_t yi, double& x, double& y) const
{
    x = (xi+0.5) * scalex + offsetx;
    y = (yi+0.5) * scaley + offsety;
}

bool GridBase::toGrid( const Point2D& point, Position& pos) const
{
    return toGrid( point.x(), point.y(), pos.x, pos.y);
}

void GridBase::fromGrid( const Position& pos, Point2D& point) const
{
    fromGrid( pos.x, pos.y, point.x(), point.y());
}

GridBase::Point2D GridBase::fromGrid( const Position& pos) const
{
    Point2D point;
    fromGrid( pos.x, pos.y, point.x(), point.y());
    return point;
}

bool GridBase::contains( const Position& pos ) const
{
    return (pos.x >= 0 && pos.x < cellSizeX 
	    && pos.y >= 0 && pos.y < cellSizeY);
}
        
GridBase::Extents GridBase::getExtents() const
{
    Eigen::Vector2d min( offsetx, offsety );
    return Extents( min, min + Eigen::Vector2d( cellSizeX * scalex, cellSizeY * scaley ) ); 
}

template<typename T>
static GridBase::Ptr readGridFromGdalHelper(std::string const& path, std::string const& band_name, int band)
{
    typename envire::Grid<T>::Ptr result = new Grid<T>();
    result->readGridData(band_name, path, band);
    return result;
}

std::pair<GridBase::Ptr, envire::FrameNode::TransformType> GridBase::readGridFromGdal(std::string const& path, std::string const& band_name, int band)
{
    GDALDataset  *poDataset;
    GDALAllRegister();
    poDataset = (GDALDataset *) GDALOpen(path.c_str(), GA_ReadOnly );
    if( poDataset == NULL )
        throw std::runtime_error("can not open file " + path);

    GDALRasterBand  *poBand;
    if(poDataset->GetRasterCount() < band)
    {
        std::stringstream strstr;
        strstr << "file " << path << " has " << poDataset->GetRasterCount() 
            << " raster bands but the band " << band << " is required";
        throw std::runtime_error(strstr.str());
    }

    poBand = poDataset->GetRasterBand(band);

    double adfGeoTransform[6];
    poDataset->GetGeoTransform(adfGeoTransform);  
    double offsetx = adfGeoTransform[0];
    double offsety = adfGeoTransform[3];

    GridBase::Ptr map;
    switch(poBand->GetRasterDataType())
    {
    case  GDT_Byte:
        map =  readGridFromGdalHelper<uint8_t>(path, band_name, band);
        break;
    case GDT_Int16:
        map =  readGridFromGdalHelper<int16_t>(path, band_name, band);
        break;
    case GDT_UInt16:
        map =  readGridFromGdalHelper<uint16_t>(path, band_name, band);
        break;
    case GDT_Int32:
        map =  readGridFromGdalHelper<int32_t>(path, band_name, band);
        break;
    case GDT_UInt32:
        map =  readGridFromGdalHelper<uint32_t>(path, band_name, band);
        break;
    case GDT_Float32:
        map =  readGridFromGdalHelper<float>(path, band_name, band);
        break;
    case GDT_Float64:
        map =  readGridFromGdalHelper<double>(path, band_name, band);
        break;
    default:
        throw std::runtime_error("enview::Grid<T>: GDT type is not supported.");  
    }

    if (adfGeoTransform[1] < 0)
        offsetx -= map->getCellSizeX() * map->getScaleX();
    if (adfGeoTransform[5] < 0)
        offsety -= map->getCellSizeY() * map->getScaleY();

    FrameNode::TransformType transform =
        FrameNode::TransformType(Eigen::Translation<double, 3>(offsetx, offsety, 0));
    return std::make_pair(map, transform);
}

void GridBase::copyBandFrom(GridBase const& source, std::string const& source_band, std::string const& _target_band)
{
    throw std::runtime_error("copyBandFrom is not implemented for this type of grid");
}

GridBase::Ptr GridBase::create(std::string const& type_name,
        size_t cellSizeX, size_t cellSizeY,
        double scale_x, double scale_y,
        double offset_x, double offset_y)
{
    Serialization so;
    so.begin();
    so.write("class", type_name);
    so.write("id", -1);
    so.write("label", "");
    so.write("immutable", false);
    so.write("cellSizeX", cellSizeX);
    so.write("cellSizeY", cellSizeY);
    so.write("scalex", scale_x);
    so.write("scaley", scale_y);
    so.write("offsetx", offset_x);
    so.write("offsety", offset_y);
    so.write("map_count", 0);
    EnvironmentItem::Ptr ptr = SerializationFactory::createObject(type_name, so);
    so.end();
    ptr->setUniqueId("");
    return boost::dynamic_pointer_cast<GridBase>(ptr);
}

bool GridBase::isCellAlignedWith(GridBase const& grid) const
{
    if (getScaleX() != grid.getScaleX() ||
        getScaleY() != grid.getScaleY() )
	return false;

    // see if the difference of the world position of the 0 grid cells is a
    // multiple of the cell scale
    Eigen::Vector3d rootDiff = 
	fromGrid( 0, 0, getEnvironment()->getRootNode() )
	 -grid.fromGrid( 0, 0, grid.getEnvironment()->getRootNode() );
    Eigen::Vector3d cellDiff = 
	(rootDiff.array() * Eigen::Array3d( 1.0/getScaleX(), 1.0/getScaleY(), 0 )).matrix();

    if( !(cellDiff.cast<int>().cast<double>() - cellDiff).isZero() )
	return false;

    return true;
}

bool GridBase::isAlignedWith(GridBase const& grid) const
{
    if (getCellSizeX() != grid.getCellSizeX() ||
        getCellSizeY() != grid.getCellSizeY() ||
        getOffsetX() != grid.getOffsetX() ||
        getOffsetY() != grid.getOffsetY())
        return false;

    FrameNode::TransformType tf = getEnvironment()->relativeTransform(this, &grid);
    base::Vector3d p(grid.getCellSizeX(), grid.getCellSizeY(), 0);
    p = tf * p;
    if (rint(p.x()) != grid.getCellSizeX() ||
            rint(p.y()) != grid.getCellSizeY())
        return false;

    return true;
}

