//
//  IrisCodecCore.hpp
//  Iris
//
//  Created by Ryan Landvater on 10/14/23.
//


#ifndef IrisCodecCore_h
#define IrisCodecCore_h
#include "IrisCodecTypes.hpp"

namespace   IrisCodec {
Version IRIS_EXPORT get_codec_version       () noexcept;

//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//      Compression Context                                                 //
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

/// Create a default compression context (see Iris Codex Context description).
Context IRIS_EXPORT create_context          () noexcept;

/// Create a IrisCodec Context that is responsible for decoding an Iris encoded file.
Context IRIS_EXPORT create_context          (const ContextCreateInfo&) noexcept;

//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//      Slide File Access                                                   //
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

/// Check basic file header signatures to see if it is an Iris Codec File
Result IRIS_EXPORT  is_iris_codec_file      (const std::string& file_path) noexcept;

/// Validate a slide file encoded using the Iris Codec
Result IRIS_EXPORT  validate_slide          (const SlideOpenInfo&) noexcept;

/// Open an IrisCodec encoded slide file
Slide IRIS_EXPORT   open_slide              (const SlideOpenInfo&) noexcept;

/// Pull the slide extent from a valid Iris Slide File
Result IRIS_EXPORT  get_slide_info          (const Slide&, SlideInfo&) noexcept;

/// Retrieve a slide tile pixel array
Buffer IRIS_EXPORT  read_slide_tile         (const SlideTileReadInfo&) noexcept;

/// Add an annotation to a slide object
Result IRIS_EXPORT  annotate_slide          (const Annotation&) noexcept;

/// Return a list of slide annotation names
Result IRIS_EXPORT get_slide_annotations    (const Slide&, Annotations&) noexcept;

//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//      Slide Temporary Cache                                               //
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

/// Create a cache / temporary codec slide file for stashing image data
Cache IRIS_EXPORT create_cache              (const CacheCreateInfo&) noexcept;

/// Read data from a Iris File Cache disk entry. It can be returned via decompression or simply copied from disk.
Buffer IRIS_EXPORT read_cache_entry         (const SlideTileReadInfo&) noexcept;

/// Write data into a Iris File Cache disk entry. It can be compressed to disk or simply copied in a bytestream form.
Result IRIS_EXPORT cache_store_entry        (const CacheStoreInfo&) noexcept;

//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//      Slide Encoder                                                       //
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
/// Slide encoder converts a vendor sldie format to
/// the Iris File Extension using the information provided within the encode slide info structure.
Encoder IRIS_EXPORT create_encoder          (EncodeSlideInfo&) noexcept;

/// Reset an active encoder object.
Result IRIS_EXPORT reset_encoder            (Encoder&) noexcept;

/// Dispatch the encoder.
Result IRIS_EXPORT dispatch_encoder         (const Encoder&) noexcept;

/// Stop an encoder immediately (safely)
Result IRIS_EXPORT interrupt_encoder        (const Encoder&) noexcept;

/// Return the encoder progress on an active encoding
Result IRIS_EXPORT get_encoder_progress     (const Encoder&, EncoderProgress&) noexcept;

/// Return an encoder object source file path.
Result IRIS_EXPORT get_encoder_src          (const Encoder&, std::string& src_string) noexcept;

/// Return an encoder object  destination directory path
Result IRIS_EXPORT get_encoder_dst_path     (const Encoder&, std::string& dst_string) noexcept;

/// Set an encoder object source file path, if not active. Attempting to alter an active encoder will fail.
Result IRIS_EXPORT set_encoder_src          (const Encoder&, const std::string&) noexcept;

/// Set the an Iris Temporary Cache file as the encoder source (ADVANCED FEATURE; read about this first).
/// This function is useful for scanner manufacturers who write into a cache and then encode a slide from that local dump.
Result IRIS_EXPORT set_encoder_src_cache    (const Encoder&, const Cache&) noexcept;

/// Set an encoder object output directory path. Attempting to alter an active encoder will fail.
Result IRIS_EXPORT set_encoder_dst_path     (const Encoder&, const std::string&) noexcept;
/// Set
}
#endif /* IrisCodecCore_h */
