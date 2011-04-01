#!/usr/bin/python

import struct

class TGAError:
    def __init__(self, text):
        self.text=text
    def __str__(self):
        return self.text

class TGAOrderError(TGAError):
    pass

class TGAFormatError(TGAError):
    pass

TYPE_INDEXED   = 1
TYPE_RGB       = 2
TYPE_GREYSCALE = 3

class TGAHeader:
    def unpack(self, header_data):
        [
            self.idfield_length,
            self.cmap_type,
            self.image_type,
            self.cmap_origin,
            self.cmap_length,
            self.cmap_entry_size,
            self.origin_x,
            self.origin_y,
            self.width,
            self.height,
            self.pixel_size,
            self.image_descriptor
        ] = struct.unpack('<BBBHHBHHHHBB', header_data)

    def pack(self):
        return struct.pack('<BBBHHBHHHHBB',
            self.idfield_length,
            self.cmap_type,
            self.image_type,
            self.cmap_origin,
            self.cmap_length,
            self.cmap_entry_size,
            self.origin_x,
            self.origin_y,
            self.width,
            self.height,
            self.pixel_size,
            self.image_descriptor
        )


class TGAFileReader:
    def __init__(self, file):
        self.header=TGAHeader()
        self.header_read=0;
        self.cmap_read=0;
        self.image_read=0;
        self.file=file;
    def readHeader(self):
        if self.header_read:
            raise TGAOrderError("Header already read")
        
        header_data=self.file.read(18)
        self.header.unpack(header_data)

        self.idfield=self.file.read(self.header.idfield_length)

        self.header_read=1
        
        if self.header.image_type != TYPE_INDEXED \
            and self.header.image_type != TYPE_RGB \
            and self.header.image_type != TYPE_GREYSCALE:
            raise TGAFormatError("Unsupported image type " + `self.header.image_type`)
        
    def readColorMap(self):
        if not self.header_read:
            raise TGAOrderError("Must read header before reading color map")
        if self.cmap_read:
            raise TGAOrderError("Color map already read")
            
        cmap_data=self.file.read(self.header.cmap_length)
        index = 0
        while index < self.header.cmap_length / 3:
            for color in range(0,3):
                self.cmap[index*3 + color] = struct.unpack("B", cmap_data[index*3+(2-color)])[0]
            index = index + 1
            
        self.cmap_read = 1
        
    def readImage(self):
        if not self.cmap_read:
            raise TGAOrderError("Must read color map before reading image")
        if self.image_read:
            raise TGAOrderError("Image already read")
        
        self.image_data = self.file.read(
            self.header.pixel_size / 8 *
            self.header.height *
            self.header.width)
        self.image_read=1
        
    def readFile(self):
        if not self.header_read:
            self.readHeader()
        if not self.cmap_read:
            self.readColorMap()
        if not self.image_read:
            self.readImage()
        
    
    def getHeader(self):
        if not self.header_read:
            raise TGAOrderError("Must read header before getHeader()")
        return self.header
    
    def getIDField(self):
        if not self.header_read:
            raise TGAOrderError("Must read header before getIDField")
        return self.idfield
    
    
    def getWidth(self):
        if not self.header_read:
            raise TGAOrderError("Must read header before getWidth()")
        return self.header.width
    
    def getHeight(self):
        if not self.header_read:
            raise TGAOrderError("Must read header before getHeight()")
        return self.header.height
    
    def getImageType(self):
        if not self.header_read:
            raise TGAOrderError("Must read header before getImageType()")
        return self.header.image_type

    def getPixelSize(self):
        if not self.header_read:
            raise TGAOrderError("Must read header before getPixelSize()")
        return self.header.pixel_size
    
    def isBottomUp(self):
        if not self.header_read:
            raise TGAOrderError("Must read header before isBottomUp()")
        if (self.header.image_descriptor & 0x20) == 0:
            return 1
        else:
            return 0
    
    def getColorMap(self):
        if not self.cmap_read:
            raise TGAOrderError("Must read color map before getColorMap()")
        return self.cmap
    
    def getImageData(self):
        if not self.image_read:
            raise TGAOrderError("Must read image before getImageData()")
        return self.image_data

    def getImage(self):
        if not self.image_read:
            raise TGAOrderError("Must read image before getImage()")

        if not self.isBottomUp():
            return self.image_data
        else:
            data = ""
            for y in range(0, self.getHeight()):
                offset=(self.getHeight()-1-y) * \
                       self.getWidth() *        \
                       self.getPixelSize()
                data = data + self.image_data[offset : offset +  
                                                       self.getWidth() *
                                                       self.getPixelSize()]
            return data


class TGAFileWriter:
    def __init__(self, file, width, height, image_type, pixel_size, image, bottom_up=0, cmap=[], idfield=""):
        self.file = file
        self.pixel_size = pixel_size
    
        self.header            = TGAHeader()
        self.header.width      = width
        self.header.height     = height
        self.header.image_type = image_type
        if (self.header.image_type == TYPE_INDEXED):
            self.header.pixel_size = 8
        else:
            self.header.pixel_size = pixel_size
        
        self.bottom_up = bottom_up
        
        if self.header.image_type != TYPE_INDEXED \
            and self.header.image_type != TYPE_RGB \
            and self.header.image_type != TYPE_GREYSCALE:
            raise TGAFormatError("Unsupported image type " + `self.header.image_type`)

        self.image   = image
        self.cmap    = cmap
        self.idfield = idfield

        self.header_written = 0;
        self.cmap_written   = 0;
        self.image_written  = 0;

    def writeHeader(self):
        if self.header_written:
            raise TGAOrderError("Header already written")
        
        self.header.idfield_length = len(self.idfield)

        if len(self.cmap) == 0:
            self.header.cmap_type = 0
        else:
            self.header.cmap_type = 1
        self.header.cmap_origin     = 0
        self.header.cmap_length     = len(self.cmap) / (self.pixel_size / 8)
        self.header.cmap_entry_size = self.pixel_size
        
        self.header.origin_x = 0
        self.header.origin_y = 0
        
        self.header.image_descriptor = 0
        if self.header.pixel_size == 32:
            self.header.image_descriptor = 8
        if not self.bottom_up:
            self.header.image_descriptor = self.header.image_descriptor | 0x20
            
        self.file.write(self.header.pack())
        self.file.write(self.idfield)
        
        self.header_written=1
    
    def writeColorMap(self):
        if not self.header_written:
            raise TGAOrderError("Must write header before writing color map")
        if self.cmap_written:
            raise TGAOrderError("Color map already written")
        
        index = 0
        while index < len(self.cmap)/3: 
            for color in range(0,3):
                print "Writing color map entry #" + `index*3 + (2 - color)`
                self.file.write(struct.pack("B", self.cmap[index*3 + (2-color)])[0])
            index = index + 1
            
        self.cmap_written = 1

    def writeImage(self):
        if not self.cmap_written:
            raise TGAOrderError("Must write color map before writing image")
        if self.image_written:
            raise TGAOrderError("Image already written")
        
        self.file.write(self.image)
        self.image_written=1
        
    def writeFile(self):
        if not self.header_written:
            self.writeHeader()
        if not self.cmap_written:
            self.writeColorMap()
        if not self.image_written:
            self.writeImage()

