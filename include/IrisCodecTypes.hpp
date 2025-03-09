/**
 * @file IrisCodecTypes.hpp
 * @author Ryan Landvater (RyanLandvater@gmail.com)
 * @brief  Iris Codec C++ Types Predeclarations
 * @version 2025.1.0
 * @date 2024-01-9
 * @copyright Copyright (c) 2022-25 Ryan Landvater
 *
 * The Iris Codec Compression Module
 * Part of the Iris Whole Slide Imaging Project
 *
 * Codec elements are defined here for use  in the implementation of the
 * Iris Codec for GPU optimized decompression routines and to allow for
 * integration of decompression pipelines with the Iris Core viewer engine.
 *
 * The codec can be optionally used outside of / without the core engine
 * for non-Iris implementations that still wish access to the compression
 * and (or) .iris file extension structure
 *
 * Use of Iris Codec and the Iris File Extension (.iris) specification follows the
 * CC BY-ND 4.0 License outlined in the Iris Digital Slide Extension File Structure Techinical Specification
 * https://creativecommons.org/licenses/by-nd/4.0/
 *
 */

#ifndef IrisCodecTypes_h
#define IrisCodecTypes_h
// Codec types rely upon the Iris Core types
// and the Core types header must be included.
#include "IrisTypes.hpp"
namespace   Iris {
using       Instance        = std::shared_ptr<struct __INTERNAL__Instance>;
using       Device          = std::shared_ptr<struct __INTERNAL__Device>;
using       Queue           = std::shared_ptr<struct __INTERNAL__Queue>;
} // END IRIS CORE PREDECLARATIONS
namespace IrisCodec {
using       Context         = std::shared_ptr<class __INTERNAL__Context>;
using       Slide           = std::shared_ptr<class __INTERNAL__Slide>;
using       Cache           = std::shared_ptr<class __INTERNAL__Cache>;
using       Encoder         = std::shared_ptr<class __INTERNAL__Encoder>;
using       Tile            = std::shared_ptr<class __INTERNAL__Tile>;
using       File            = std::shared_ptr<class __INTERNAL__File>;
using       Version         = Iris::Version;
using       Result          = Iris::Result;
using       Buffer          = Iris::Buffer;
using       Extent          = Iris::Extent;
using       Format          = Iris::Format;
using       AnnotationTypes = Iris::AnnotationTypes;
using       Annotation      = Iris::Annotation;
using       Annotations     = Iris::Annotations;
using       AnnotationGroup = Iris::AnnotationGroup;
using       Mutex           = std::mutex;
using       Offset          = uint64_t;
using       Size            = uint64_t;
struct ContextCreateInfo {
    Iris::Device    device                  = nullptr;
};
/// Form of encoding used to generate compressed tile bytestreams
enum Encoding : uint8_t {
    TILE_ENCODING_UNDEFINED                 = 0,
    TILE_ENCODING_IRIS,
    TILE_ENCODING_JPEG,
    TILE_ENCODING_AVIF,
    ENCODING_DEFAULT                        = TILE_ENCODING_JPEG //v2025.1 - will change
};
enum MetadataType : uint8_t {
    METADATA_UNDEFINED                      = 0,
    METADATA_I2S,
    METADATA_DICOM,
    METADATA_FREE_TEXT                      = METADATA_I2S,
};
struct Attributes :
public std::unordered_map<std::string, std::u8string> {
    MetadataType    type                    = METADATA_UNDEFINED;
    uint16_t        version                 = 0;
};
enum ImageEncoding : uint8_t {
    IMAGE_ENCODING_UNDEFINED                = 0,
    IMAGE_ENCODING_PNG                      = 1,
    IMAGE_ENCODING_JPEG                     = 2,
    IMAGE_ENCODING_AVIF                     = 3,
};
enum ImageOrientation : uint16_t {
    ORIENTATION_0                           = 0,
    ORIENTATION_90                          = 90,
    ORIENTATION_180                         = 180,
    ORIENTATION_270                         = 270,
    ORIENTATION_minus_90                    = ORIENTATION_270,
    ORIENTATION_minus_180                   = ORIENTATION_180,
    ORIENTATION_minus_270                   = ORIENTATION_90,
};
struct Image {
    using           Orientation             = ImageOrientation;
    std::string     title;
    Buffer          bytes                   = nullptr;
    uint32_t        width                   = 0;
    uint32_t        height                  = 0;
    Format          format                  = Iris::FORMAT_UNDEFINED;
    Orientation     orientation             = ORIENTATION_0;
};
/// Slide metadata containing information about the Iris File Extension slide file.
struct Metadata {
    /// List of associated / ancillary image labels describing the associated image (*eg. Label, Thumbnail*)
    using ImageLabels                       = std::set<std::string>;
    /// List of annotation identifiers within the slide (unique annotation identifiers)
    using AnnotationIDs                     = std::set<uint32_t>;
    /// List of annotation grouping names describing the group of annotations (*eg. handwriting; nuclei*)
    using AnnotationGroups                  = std::set<std::string>;
    
    /// Iris Codec Version used to encode the file, if Iris Codec encoded it. Otherwise leave this blank
    Version         codec                   = {0,0,0};
    /// Metadata Attributes containing key-value pairs
    Attributes      attributes;
    /// List of associated images
    ImageLabels     associatedImages;
    /// ICC color profile, if contained within the file
    std::string     ICC_profile;
    /// Iris slide annotations
    AnnotationIDs   annotations;
    /// Iris slide annotation groups
    AnnotationGroups annotationGroups;
    /// Microns per pixel (um per pixel) at layer 0 / the lowest resolution layer (0.f if no um scale)
    float           micronsPerPixexl        = 0.f;
    /// Magnification coefficient used to convert scale to physical microscopic magnfication (0.f if not included)
    float           magnification           = 0.f;
};

/// Information needed to open a local Iris Encoded (.iris) file with optional defaults assigned.
struct SlideOpenInfo {
    std::string     filePath;
    Context         context                 = nullptr;
    bool            writeAccess             = false;
};

/// Iris Encoded File (.iris) metadata information
struct SlideInfo {
    Format          format                  = Iris::FORMAT_UNDEFINED;
    Encoding        encoding                = TILE_ENCODING_UNDEFINED;
    Extent          extent;
    Metadata        metadata;
};
struct SlideTileReadInfo {
    Slide           slide                   = NULL;
    uint32_t        layerIndex              = 0;
    uint32_t        tileIndex               = 0;
    Buffer          optionalDestination     = NULL;
    Format          desiredFormat           = Iris::FORMAT_R8G8B8A8;
};
// MARK: - CACHE TYPE DEFINITIONS
struct CacheCreateInfo {
    Context         context                 = nullptr;
};
struct CacheTileReadInfo {
    Cache           cache                   = NULL;
    uint32_t        layerIndex              = 0;
    uint32_t        tileIndex               = 0;
    Buffer          optionalDestination     = NULL;
    Format          desiredFormat           = Iris::FORMAT_R8G8B8A8;
};
struct CacheStoreInfo {
    Cache           cache                   = NULL;
    uint32_t        layerIndex              = 0;
    uint32_t        tileIndex               = 0;
    Buffer          source                  = NULL;
    size_t          expandedSize            = 0;
};

// MARK: - ENCODER TYPE DEFINITIONS
/// Image encoding quality [0-100]. Maps to JPEG and AVIF quality standard
using               Quality                 = uint16_t;
constexpr Quality   QUALITY_DEFAULT         = 90;

/// Available Chroma-Subsampling options. More information at:
/// https://en.wikipedia.org/wiki/Chroma_subsampling
enum Subsampling : uint8_t {
    SUBSAMPLE_444, // Lossless
    SUBSAMPLE_422,
    SUBSAMPLE_420,
    SUBSAMPLE_DEFAULT                       = SUBSAMPLE_422,
};

/// Current status of the encoder object.
enum EncoderStatus {
    ENCODER_INACTIVE,
    ENCODER_ACTIVE,
    ENCODER_ERROR,
    ENCODER_SHUTDOWN,
};
struct EncodeSlideInfo {
    std::string     srcFilePath;
    std::string     dstFilePath;
    Format          srcFormat               = Iris::FORMAT_UNDEFINED;
    Encoding        desiredEncoding         = TILE_ENCODING_UNDEFINED;
    Format          desiredFormat           = Iris::FORMAT_UNDEFINED;
    Context         context                 = NULL;
};
struct EncoderProgress {
    EncoderStatus   status                  = ENCODER_INACTIVE;
    float           progress                = 0.f;
    std::string     dstFilePath;
    std::string     errorMsg;
};
} // END IRIS CODEC NAMESPACE
#endif /* IrisCodecTypes_h */
