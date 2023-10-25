#ifndef slic3r_EmbossShape_hpp_
#define slic3r_EmbossShape_hpp_

#include <string>
#include <optional>
#include <memory> // unique_ptr
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/archives/binary.hpp>
#include "Point.hpp" // Transform3d
#include "ExPolygon.hpp"
#include "ExPolygonSerialize.hpp"
#include "nanosvg/nanosvg.h" // NSVGimage

namespace Slic3r {

struct EmbossProjection
{
    // Emboss depth, Size in local Z direction
    double depth = 1.; // [in loacal mm] 
    // NOTE: User should see and modify mainly world size not local

    // Flag that result volume use surface cutted from source objects
    bool use_surface = false;

    // enum class Align {
    //     left,
    //     right,
    //     center,
    //     top_left,
    //     top_right,
    //     top_center,
    //     bottom_left,
    //     bottom_right,
    //     bottom_center
    // };
    //// change pivot of volume
    //// When not set, center is used and is not stored
    // std::optional<Align> align;

    // compare TextStyle
    bool operator==(const EmbossProjection &other) const {
        return depth == other.depth && use_surface == other.use_surface;
    }

    // undo / redo stack recovery
    template<class Archive> void serialize(Archive &ar) { ar(depth, use_surface); }
};

// Help structure to identify expolygons grups
// e.g. emboss -> per glyph -> identify character
struct ExPolygonsWithId
{ 
    // Identificator for shape
    // In text it separate letters and the name is unicode value of letter
    // Is svg it is id of path
    unsigned id;

    // shape defined by integer point contain only lines
    // Curves are converted to sequence of lines
    ExPolygons expoly;

    // flag whether expolygons are fully healed(without duplication)
    bool is_healed = true;
};
using ExPolygonsWithIds = std::vector<ExPolygonsWithId>;

/// <summary>
/// Contain plane shape information to be able emboss it and edit it
/// </summary>
struct EmbossShape 
{
    // shapes to to emboss separately over surface
    ExPolygonsWithIds shapes_with_ids;

    // scale of shape, multiplier to get 3d point in mm from integer shape
    double scale = SCALING_FACTOR;

    // Define how to emboss shape
    EmbossProjection projection;

    // !!! Volume stored in .3mf has transformed vertices.
    // (baked transformation into vertices position)
    // Only place for fill this is when load from .3mf
    // This is correction for volume transformation
    // Stored_Transform3d * fix_3mf_tr = Transform3d_before_store_to_3mf
    std::optional<Slic3r::Transform3d> fix_3mf_tr;

    struct SvgFile {
        // File(.svg) path on local computer 
        // When empty can't reload from disk
        std::string path;

        // File path into .3mf(.zip)
        // When empty svg is not stored into .3mf file yet.
        // and will create dialog to delete private data on save.
        std::string path_in_3mf;

        // Loaded svg file data.
        // !!! It is not serialized on undo/redo stack 
        std::shared_ptr<NSVGimage> image = nullptr;

        // Loaded string data from file
        std::shared_ptr<std::string> file_data = nullptr;
    };    
    SvgFile svg_file;

    // flag whether during cration of union expolygon final shape was fully correct
    // correct mean without selfintersection and duplicate(double) points
    bool is_healed = true;

    // undo / redo stack recovery
    template<class Archive> void save(Archive &ar) const
    {
        ar(shapes_with_ids, scale, projection, svg_file.path, svg_file.path_in_3mf);
        cereal::save(ar, fix_3mf_tr);
    }
    template<class Archive> void load(Archive &ar)
    {
        ar(shapes_with_ids, scale, projection, svg_file.path, svg_file.path_in_3mf);
        cereal::load(ar, fix_3mf_tr);
    }
};

} // namespace Slic3r

// Serialization through the Cereal library
namespace cereal {
template<class Archive> void serialize(Archive &ar, Slic3r::ExPolygonsWithId &o) { ar(o.id, o.expoly); }
}; // namespace cereal

#endif // slic3r_EmbossShape_hpp_
