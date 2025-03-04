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
Version     get_codec_version       () noexcept;

//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//      Compression Context                                                 //
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

/// Create a default compression context (see Iris Codex Context description).
Context     create_context          () noexcept;

/// Create a IrisCodec Context that is responsible for decoding an Iris encoded file.
Context     create_context          (const ContextCreateInfo&) noexcept;

//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//      Slide File Access                                                   //
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

/// Check basic file header signatures to see if it is an Iris Codec File
Result      is_iris_codec_file      (const std::string& file_path) noexcept;

/// Validate a slide file encoded using the Iris Codec
Result      validate_slide          (const SlideOpenInfo&) noexcept;

/// Open an IrisCodec encoded slide file
Slide       open_slide              (const SlideOpenInfo&) noexcept;

/// Pull the slide extent from a valid Iris Slide File
Result      get_slide_info          (const Slide&, SlideInfo&) noexcept;

/// Retrieve a slide tile pixel array
Buffer      read_slide_tile         (const SlideTileReadInfo&) noexcept;

/// Add an annotation to a slide object
Result      annotate_slide          (const Annotation&) noexcept;

Result      get_slide_annotations   (const Slide&, Annotations&) noexcept;

//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//      Slide Temporary Cache                                               //
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
// Create a cache / temporary codec slide file for stashing image data
Cache       create_cache            (const CacheCreateInfo&) noexcept;
// Read
Buffer      read_cache_entry        (const SlideTileReadInfo&) noexcept;

Result      cache_store_entry       (const CacheStoreInfo&) noexcept;

//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//      Slide Encoder                                                       //
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
/// Slide encoder converts a vendor sldie format to
/// the Iris File Extension using the information provided within the encode slide info structure.
Encoder     create_encoder          (EncodeSlideInfo&) noexcept;
Result      reset_encoder           (Encoder&) noexcept;
/// Dispatch the encoder.
Result      dispatch_encoder        (const Encoder&) noexcept;
Result      interrupt_encoder       (const Encoder&) noexcept;
Result      get_encoder_progress    (const Encoder&, EncoderProgress&) noexcept;
Result      get_encoder_src         (const Encoder&, std::string& src_string) noexcept;
Result      get_encoder_dst_path    (const Encoder&, std::string& dst_string) noexcept;
Result      set_encoder_src         (const Encoder&, const std::string&) noexcept;
Result      set_encoder_dst_path    (const Encoder&, const std::string&) noexcept;

//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//      File Convenience Wrapper                                            //
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//  These file access methods are offered for convenience
/// Create a new file for writing
File        create_file             (const FileCreateInfo&);

/// Open a file for read or read-write access.
File        open_file               (const FileOpenInfo&);

/// Resize a file
Result      resize_file             (const File&, const FileResizeInfo&);
}
#endif /* IrisCodecCore_h */
