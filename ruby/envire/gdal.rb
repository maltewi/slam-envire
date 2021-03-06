# Part of this code has been taken from the gdal-helper gem from Jay Cable <jay@spruceboy.net>
# Copyright:: Copyright (c) 2010 Jay Cable
# License::   Distributes under the same terms as Ruby.

begin
  require "gdal/gdal"
  require "gdal/gdalconst"
  require 'gdal/osr'
rescue LoadError=>e
  require "gdal"
  require "gdalconst"
  require 'osr'
end

module Gdal
  # Does type mappings from gdal to ruby - pretty basic, and needs to be expanded so other types can be done, like for example
  # 32 bit integer types, 16 bit types, double floating bit types, unsigned int types, etc..
  def self.data_type_from_gdal(data_type)
    case (data_type)
      when Gdal::Gdalconst::GDT_UNKNOWN; return null
      when Gdal::Gdalconst::GDT_BYTE; return String
      when Gdal::Gdalconst::GDT_UINT16 ; return Integer
      when Gdal::Gdalconst::GDT_INT16; return Integer
      when Gdal::Gdalconst::GDT_UINT32; return Integer
      when Gdal::Gdalconst::GDT_INT32; return Integer
      when Gdal::Gdalconst::GDT_FLOAT32; return Float
      when Gdal::Gdalconst::GDT_FLOAT64; return Float
      when Gdal::Gdalconst::GDT_CINT16; return Integer
      when Gdal::Gdalconst::GDT_CINT32; return Float
      when Gdal::Gdalconst::GDT_CFLOAT32; return Float
      when Gdal::Gdalconst::GDT_CFLOAT64; return Float
      else raise ArgumentError("Unknown data type.. not sure what to do here folks", caller)
    end
  end
  
  # Does type mappings from ruby to gdal - pretty basic, and needs to be expanded so other types can be done, like for example
  # 32 bit integer types, 16 bit types, double floating bit types, unsigned int types, etc..
  def self.data_type_to_gdal ( data_type )
    return Gdal::Gdalconst::GDT_BYTE if (data_type == String )
    return Gdal::Gdalconst::GDT_INT16 if (data_type == Integer )
    return Gdal::Gdalconst::GDT_FLOAT32 if (data_type == Float )
    raise ArgumentError, "#{data_type} is not a valid (String|Integer|Float) data type.. not sure what to do here folks", caller
  end

  #--
  # This is just for reference - don't rdoc this stuff..
  # string.pack notes - for refrence for the function below.
  #  Format | Returns | Function
  #-------+---------+-----------------------------------------
  #  A    | String  | with trailing nulls and spaces removed
  #-------+---------+-----------------------------------------
  #  a    | String  | string
  #-------+---------+-----------------------------------------
  #  B    | String  | extract bits from each character (msb first)
  #-------+---------+-----------------------------------------
  #  b    | String  | extract bits from each character (lsb first)
  #-------+---------+-----------------------------------------
  #  C    | Fixnum  | extract a character as an unsigned integer
  #-------+---------+-----------------------------------------
  #  c    | Fixnum  | extract a character as an integer
  #-------+---------+-----------------------------------------
  #  d,D  | Float   | treat sizeof(double) characters as
  #       |         | a native double
  #-------+---------+-----------------------------------------
  #  E    | Float   | treat sizeof(double) characters as
  #       |         | a double in little-endian byte order
  #-------+---------+-----------------------------------------
  #  e    | Float   | treat sizeof(float) characters as
  #       |         | a float in little-endian byte order
  #-------+---------+-----------------------------------------
  #  f,F  | Float   | treat sizeof(float) characters as
  #       |         | a native float
  #-------+---------+-----------------------------------------
  #  G    | Float   | treat sizeof(double) characters as
  #       |         | a double in network byte order
  #-------+---------+-----------------------------------------
  #  g    | Float   | treat sizeof(float) characters as a
  #       |         | float in network byte order
  #-------+---------+-----------------------------------------
  #  H    | String  | extract hex nibbles from each character
  #       |         | (most significant first)
  #-------+---------+-----------------------------------------
  #  h    | String  | extract hex nibbles from each character
  #       |         | (least significant first)
  #-------+---------+-----------------------------------------
  #  I    | Integer | treat sizeof(int) (modified by _)
  #       |         | successive characters as an unsigned
  #       |         | native integer
  #-------+---------+-----------------------------------------
  #  i    | Integer | treat sizeof(int) (modified by _)
  #       |         | successive characters as a signed
  #       |         | native integer
  #-------+---------+-----------------------------------------
  #  L    | Integer | treat four (modified by _) successive
  #       |         | characters as an unsigned native
  #       |         | long integer
  #-------+---------+-----------------------------------------
  #  l    | Integer | treat four (modified by _) successive
  #       |         | characters as a signed native
  #       |         | long integer
  #-------+---------+-----------------------------------------
  #  M    | String  | quoted-printable
  #-------+---------+-----------------------------------------
  #  m    | String  | base64-encoded
  #-------+---------+-----------------------------------------
  #  N    | Integer | treat four characters as an unsigned
  #       |         | long in network byte order
  #-------+---------+-----------------------------------------
  #  n    | Fixnum  | treat two characters as an unsigned
  #       |         | short in network byte order
  #-------+---------+-----------------------------------------
  #  P    | String  | treat sizeof(char *) characters as a
  #       |         | pointer, and  return \emph{len} characters
  #       |         | from the referenced location
  #-------+---------+-----------------------------------------
  #  p    | String  | treat sizeof(char *) characters as a
  #       |         | pointer to a  null-terminated string
  #-------+---------+-----------------------------------------
  #  Q    | Integer | treat 8 characters as an unsigned
  #       |         | quad word (64 bits)
  #-------+---------+-----------------------------------------
  #  q    | Integer | treat 8 characters as a signed
  #       |         | quad word (64 bits)
  #-------+---------+-----------------------------------------
  #  S    | Fixnum  | treat two (different if _ used)
  #       |         | successive characters as an unsigned
  #       |         | short in native byte order
  #-------+---------+-----------------------------------------
  #  s    | Fixnum  | Treat two (different if _ used)
  #       |         | successive characters as a signed short
  #       |         | in native byte order
  #-------+---------+-----------------------------------------
  #  U    | Integer | UTF-8 characters as unsigned integers
  #-------+---------+-----------------------------------------
  #  u    | String  | UU-encoded
  #-------+---------+-----------------------------------------
  #  V    | Fixnum  | treat four characters as an unsigned
  #       |         | long in little-endian byte order
  #-------+---------+-----------------------------------------
  #  v    | Fixnum  | treat two characters as an unsigned
  #       |         | short in little-endian byte order
  #-------+---------+-----------------------------------------
  #  w    | Integer | BER-compressed integer (see Array.pack)
  #-------+---------+-----------------------------------------
  #  X    | ---     | skip backward one character
  #-------+---------+-----------------------------------------
  #  x    | ---     | skip forward one character
  #-------+---------+-----------------------------------------
  #  Z    | String  | with trailing nulls removed
  #       |         | upto first null with *
  #-------+---------+-----------------------------------------
  #  @    | ---     | skip to the offset given by the
  #       |         | length argument
  #-------+---------+-----------------------------------------
  #++
  #unpacks the data
  def self.unpack(data, data_type)
    pack_template = case (data_type)
      when ::Gdal::Gdalconst::GDT_UNKNOWN;raise ArgumentError, "GDT_UNKNOWN has no storage template.. not sure what to do here folks", caller
      when ::Gdal::Gdalconst::GDT_BYTE; 'C'
      when ::Gdal::Gdalconst::GDT_UINT16;'S'
      when ::Gdal::Gdalconst::GDT_INT16;'s'
      when ::Gdal::Gdalconst::GDT_UINT32; 'I'
      when ::Gdal::Gdalconst::GDT_INT32; 'i'
      when ::Gdal::Gdalconst::GDT_FLOAT32; 'f'
      when ::Gdal::Gdalconst::GDT_FLOAT64; 'D'
      when ::Gdal::Gdalconst::GDT_CINT16; ''    #What are these?
      when ::Gdal::Gdalconst::GDT_CINT32; ''    #What are these?
      when ::Gdal::Gdalconst::GDT_CFLOAT32; ''  #What are these?
      when ::Gdal::Gdalconst::GDT_CFLOAT64; '' #What are these?
      else raise ArgumentError("Unknown data type.. not sure what to do here folks", caller)
    end
    return data.unpack("#{pack_template}*")
  end
  
  #--
  # dont rdoc this stuff.. just notes from pack for my own quick reference..
  #Notes for pack..
  # #Directives for pack.
  #
  #Directive    Meaning
  #---------------------------------------------------------------
  #    @     |  Moves to absolute position
  #    A     |  ASCII string (space padded, count is width)
  #    a     |  ASCII string (null padded, count is width)
  #    B     |  Bit string (descending bit order)
  #    b     |  Bit string (ascending bit order)
  #    C     |  Unsigned char
  #    c     |  Char
  #    D, d  |  Double-precision float, native format
  #    E     |  Double-precision float, little-endian byte order
  #    e     |  Single-precision float, little-endian byte order
  #    F, f  |  Single-precision float, native format
  #    G     |  Double-precision float, network (big-endian) byte order
  #    g     |  Single-precision float, network (big-endian) byte order
  #    H     |  Hex string (high nibble first)
  #    h     |  Hex string (low nibble first)
  #    I     |  Unsigned integer
  #    i     |  Integer
  #    L     |  Unsigned long
  #    l     |  Long
  #    M     |  Quoted printable, MIME encoding (see RFC2045)
  #    m     |  Base64 encoded string
  #    N     |  Long, network (big-endian) byte order
  #    n     |  Short, network (big-endian) byte-order
  #    P     |  Pointer to a structure (fixed-length string)
  #    p     |  Pointer to a null-terminated string
  #    Q, q  |  64-bit number
  #    S     |  Unsigned short
  #    s     |  Short
  #    U     |  UTF-8
  #    u     |  UU-encoded string
  #    V     |  Long, little-endian byte order
  #    v     |  Short, little-endian byte order
  #    w     |  BER-compressed integer\fnm
  #    X     |  Back up a byte
  #    x     |  Null byte
  #    Z     |  Same as ``a'', except that null is added with *
  #++
  # packs data in prep to write to gdal..
  def self.pack(data, data_type)
    pack_template = case(data_type)
      when ::Gdal::Gdalconst::GDT_UNKNOWN;raise ArgumentError, "GDT_UNKNOWN has no storage template.. not sure what to do here folks", caller
      when ::Gdal::Gdalconst::GDT_BYTE; 'c'
      when ::Gdal::Gdalconst::GDT_UINT16;'S'
      when ::Gdal::Gdalconst::GDT_INT16;'s'
      when ::Gdal::Gdalconst::GDT_UINT32; 'I'
      when ::Gdal::Gdalconst::GDT_INT32; 'i'
      when ::Gdal::Gdalconst::GDT_FLOAT32; 'f'
      when ::Gdal::Gdalconst::GDT_FLOAT64; 'D'
      when ::Gdal::Gdalconst::GDT_CINT16; ''    #What are these? Complex types?
      when ::Gdal::Gdalconst::GDT_CINT32; ''    #What are these?
      when ::Gdal::Gdalconst::GDT_CFLOAT32; ''  #What are these?
      when ::Gdal::Gdalconst::GDT_CFLOAT64; '' #What are these?
      else raise ArgumentError, "Unknown datatype.. not sure what to do here folks", caller
    end
    raise(ArgumentError, "Complex type requested, but no complex type handling.. not sure what to do here folks", caller) if ( pack_template == '')
    return data.pack("#{pack_template}*")
  end

  module Gdal
    class Band
      def read(start_x, start_y, width_x, width_y)
        return ::Gdal.unpack(read_raster(start_x,start_y,width_x,width_y), self.DataType)
      end

      #Writes data
      def write(start_x, start_y, width_x, width_y, data)
        return write_raster(start_x,start_y,width_x,width_y, ::Gdal.pack(data, self.DataType))
      end

      #returns the datatype
      def data_type()
        ::Gdal.data_type_from_gdal(self.DataType)
      end
  
      #returns the datatype as a string in a gdal like manner
      def data_type_pretty
        type_string =
          case(@band.DataType)
          when Gdal::Gdalconst::GDT_UNKNOWN; 'GDT_UNKNOWN'
          when Gdal::Gdalconst::GDT_BYTE; 'GDT_BYTE'
          when Gdal::Gdalconst::GDT_UINT16;'GDT_UINT16'
          when Gdal::Gdalconst::GDT_INT16;'GDT_INT16'
          when Gdal::Gdalconst::GDT_UINT32; 'GDT_UINT32'
          when Gdal::Gdalconst::GDT_INT32; 'GDT_INT32'
          when Gdal::Gdalconst::GDT_FLOAT32; 'IDT_FLOAT32'
          when Gdal::Gdalconst::GDT_FLOAT64; 'GDT_FLOAT64'
          when Gdal::Gdalconst::GDT_CINT16; 'GDT_CINT16'   
          when Gdal::Gdalconst::GDT_CINT32; 'GDT_CINT32'  
          when Gdal::Gdalconst::GDT_CFLOAT32; 'GDT_CFLOAT32' 
          when Gdal::Gdalconst::GDT_CFLOAT64; 'GDT_CFLOAT64'
          else raise ArgumentError("Unknown data type.. not sure what to do here folks", caller)
          end
        type_string
      end
    end
    
    class Dataset
      #number of bands
      def number_of_bands()
        self.RasterCount
      end

      def xsize
        self.RasterXSize
      end

      def ysize
        self.RasterYSize
      end
      
      def band(band_index)
          @bands ||= Array.new
          @bands[band_index] ||= get_raster_band(band_index)
      end

      def read_band(band_index, x = 0, y = 0, w = xsize, h = ysize)
          b = band(band_index)
          b.read(x, y, w, h)
      end

      def write_band(band_index, data, x = 0, y = 0, w = xsize, h = ysize)
          b = band(band_index)
          b.write(x, y, w, h, data)
      end

      def apply_geo_transform(x, y)
          geo_t = get_geo_transform
          return [geo_t[0] + x * geo_t[1] + y * geo_t[2],
              geo_t[3] + x * geo_t[4] + y * geo_t[5]]
      end
    end
  end
end

