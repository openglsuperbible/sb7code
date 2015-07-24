#ifndef __SB6KTX_H__
#define __SB6KTX_H__

namespace sb7
{

namespace ktx
{

namespace file
{

struct header
{
    unsigned char       identifier[12];
    unsigned int        endianness;
    unsigned int        gltype;
    unsigned int        gltypesize;
    unsigned int        glformat;
    unsigned int        glinternalformat;
    unsigned int        glbaseinternalformat;
    unsigned int        pixelwidth;
    unsigned int        pixelheight;
    unsigned int        pixeldepth;
    unsigned int        arrayelements;
    unsigned int        faces;
    unsigned int        miplevels;
    unsigned int        keypairbytes;
};

union keyvaluepair
{
    unsigned int        size;
    unsigned char       rawbytes[4];
};

unsigned int load(const char * filename, unsigned int tex = 0);
bool save(const char * filename, unsigned int target, unsigned int tex);

}

}

}

#endif /* __SB6KTX_H__ */
