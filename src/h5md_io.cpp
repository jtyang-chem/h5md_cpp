#include <H5Cpp.h>
#include "h5md_io.h"
#include <iostream>
#include <vector>

using namespace H5;

H5MDWriter::H5MDWriter(const std::string &filename, const std::string &method, const bool &isPbc)
{
    this->method = method;
    if (method == "w")
    {
        this->filename = filename;
        this->file = createH5mdFile(filename);
        this->isPbc = isPbc;
    }
    else
    {
        throw std::runtime_error("method must be 'w' for now");
    }
}

H5MDWriter::~H5MDWriter()
{
}

/// @brief Creates an HDF5 file with the specified name and initializes it
/// @param fileName
/// @param group_name
/// @return file handle
/// @throws H5::Exception if file creation fails
H5File H5MDWriter::createH5mdFile(const std::string &fileName)
{
    // H5File file = createH5File(fileName);
    H5File file = H5File(fileName, H5F_ACC_TRUNC);

    Group h5md_group = file.createGroup("/h5md");

    // add version attribute
    hsize_t dims[1] = {2};
    int version[2] = {1, 0}; // Example version
    DataSpace version_space(1, dims);
    Attribute version_attr = h5md_group.createAttribute("version", PredType::NATIVE_INT, version_space);
    version_attr.write(PredType::NATIVE_INT, version);

    // author and creator
    Group author_group = h5md_group.createGroup("author");
    std::string author = "jtyang";
    DataSpace scalar_space = DataSpace(H5S_SCALAR);
    StrType str_type(PredType::C_S1, author.size() + 1);
    str_type.setStrpad(H5T_STR_NULLTERM);
    Attribute author_attr = author_group.createAttribute("name", str_type, scalar_space);
    author_attr.write(str_type, author.c_str());

    Group creator_group = h5md_group.createGroup("creator");
    std::string creator = "qbics";
    DataSpace creator_space = DataSpace(H5S_SCALAR);
    StrType creator_type(PredType::C_S1, creator.size() + 1);
    creator_type.setStrpad(H5T_STR_NULLTERM);
    Attribute creator_attr = creator_group.createAttribute("name", creator_type, creator_space);
    creator_attr.write(creator_type, creator.c_str());

    std::string creator_version = "0.4";
    DataSpace version_space_creator = DataSpace(H5S_SCALAR);
    StrType version_type(PredType::C_S1, creator_version.size() + 1);
    version_type.setStrpad(H5T_STR_NULLTERM);
    Attribute version_attr_creator = creator_group.createAttribute("version", version_type, version_space_creator);
    version_attr_creator.write(version_type, creator_version.c_str());

    return file;
}

void H5MDWriter::append_one_step(
                                 const std::vector<std::vector<double>> &step_data,
                                 double time_val,
                                 int step_val,
                                 std::vector<int> &species)
{
    H5File file = this->file; // Use the class member file handle
    const hsize_t nparticles = 2;
    const hsize_t ndim = 3;

    std::string particle_path = "/particles";
    std::string position_path = "/particles/beads/position/value";
    std::string time_path = "/particles/beads/position/time";
    std::string step_path = "/particles/beads/position/step";

    DataSet value_dataset;
    hsize_t dims[3], maxdims[3] = {H5S_UNLIMITED, nparticles, ndim};
    hsize_t chunk_dims[3] = {1, nparticles, ndim};
    bool first_write = false;

    DataSet time_dataset;
    DataSet step_dataset;
    hsize_t time_dims[1], step_dims[1];
    hsize_t time_chunk_dims[1] = {1};
    hsize_t step_chunk_dims[1] = {1};

    if (!H5Lexists(file.getId(), "particles", H5P_DEFAULT))
    {
        // First time writing: create dataset
        Group particles_group = file.createGroup("/particles");
        Group beads_group = particles_group.createGroup("beads");
        Group position_group = beads_group.createGroup("position");
        printf("<== Creating dataset... ==>\n");
        hsize_t start_dims[3] = {0, nparticles, ndim};
        DataSpace space(3, start_dims, maxdims);
        DSetCreatPropList plist;
        plist.setChunk(3, chunk_dims);
        plist.setDeflate(6);
        value_dataset = file.createDataSet(position_path, PredType::IEEE_F64LE, space, plist);

        // Create species dataset
        printf("<== Creating species dataset... ==>\n");
        hsize_t species_dims[1] = {nparticles};
        DataSpace species_space(1, species_dims);
        plist.setChunk(1, chunk_dims);
        plist.setDeflate(6);
        DataSet species_dataset = beads_group.createDataSet("species", PredType::NATIVE_INT, species_space, plist);
        species_dataset.write(species.data(), PredType::NATIVE_INT);

        // Create step dataset
        printf("<== Creating step dataset... ==>\n");
        hsize_t step_start_dims[1] = {0};
        hsize_t step_max_dims[1] = {H5S_UNLIMITED};
        DataSpace step_space(1, step_start_dims, step_max_dims);
        plist.setChunk(1, step_chunk_dims);
        plist.setDeflate(6);
        step_dataset = file.createDataSet(step_path, PredType::NATIVE_INT, step_space, plist);


        
        // nopbc info
        printf("<==  deal with PBC info... ==>\n");
        Group box_group = beads_group.createGroup("box");

        // dimension
        hsize_t attr_dim[1] = {1};
        const int dim_boundary = 3;
        DataSpace dim_space = DataSpace(H5S_SCALAR);
        ;
        Attribute dimension_attr = box_group.createAttribute("dimension", PredType::NATIVE_INT, dim_space);
        dimension_attr.write(PredType::NATIVE_INT, &dim_boundary);
        // boundary
        hsize_t attr_boundary[1] = {3};
        DataSpace boundary_space(1, attr_boundary);
        StrType str_type(PredType::C_S1, H5T_VARIABLE);
        Attribute boundary_attr = box_group.createAttribute("boundary", str_type, boundary_space);
        const char *boundary_data[] = {"none", "none", "none"};
        boundary_attr.write(str_type, boundary_data);
        // If all values in boundary are none, edges may be omitted.
    }
    else
    {
        printf("Appending to dataset... \n");
        value_dataset = file.openDataSet(position_path);
        step_dataset = file.openDataSet(step_path);
    }

    printf("<== Appending data... ==>\n");
    // Get current dims and extend
    DataSpace filespace = value_dataset.getSpace();
    filespace.getSimpleExtentDims(dims);

    hsize_t new_dims[3] = {dims[0] + 1, nparticles, ndim};
    value_dataset.extend(new_dims);

    // Select hyperslab for the new step saving
    DataSpace new_filespace = value_dataset.getSpace();
    hsize_t offset[3] = {dims[0], 0, 0};
    hsize_t count[3] = {1, nparticles, ndim};
    new_filespace.selectHyperslab(H5S_SELECT_SET, count, offset);

    // Memory space for 1 step
    DataSpace memspace(3, count);

    // Flatten data
    std::vector<double> flat_data(nparticles * ndim);
    for (size_t i = 0; i < nparticles; ++i)
        for (size_t j = 0; j < ndim; ++j)
            flat_data[i * ndim + j] = step_data[i][j];

    value_dataset.write(flat_data.data(), PredType::NATIVE_DOUBLE, memspace, new_filespace);
    // // === Append to step ===
    printf("<== Appending step... ==>\n");
    hsize_t t_dims[1];
    step_dataset.getSpace().getSimpleExtentDims(t_dims);

    hsize_t new_tdims[1] = {t_dims[0] + 1};
    step_dataset.extend(new_tdims);

    DataSpace sspace = step_dataset.getSpace();
    hsize_t toffset[1] = {t_dims[0]};
    hsize_t tcount[1] = {1};

    sspace.selectHyperslab(H5S_SELECT_SET, tcount, toffset);

    DataSpace smem(H5S_SCALAR);
    step_dataset.write(&step_val, PredType::NATIVE_INT, smem, sspace);
}

// h5md's flush function has some problem, 
// so we have to close and open the file again.
// note the file cannot be read while it is open in program,
// though contents has been flushed to disk.
void H5MDWriter::flush()
{
    printf("==> h5md Flushing... ==>\n");
    this->file.close();
    this->file.openFile(this->filename, H5F_ACC_RDWR);
}