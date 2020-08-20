#include <limits>

#include "catch.hpp"

#include "data_types.h"
#include "hdf5_particle.h"
#include "material.h"
#include "mpi_datatypes.h"
#include "mpi_datatypes_twophase.h"
#include "particle.h"
#include "particle_twophase.h"

#ifdef USE_MPI
//! \brief Check particle class for 1D case
TEST_CASE("MPI HDF5 TwoPhase Particle is checked",
          "[particle][mpi][hdf5][2Phase]") {
  // Dimension
  const unsigned Dim = 3;
  // Tolerance
  const double Tolerance = 1.E-7;

  // Check MPI transfer of HDF5 Particle
  SECTION("MPI HDF5 TwoPhase Particle") {
    mpm::Index id = 0;

    mpm::HDF5ParticleTwoPhase h5_particle;
    h5_particle.id = 13;
    h5_particle.mass = 501.5;

    h5_particle.pressure = 125.75;

    Eigen::Vector3d coords;
    coords << 1., 2., 3.;
    h5_particle.coord_x = coords[0];
    h5_particle.coord_y = coords[1];
    h5_particle.coord_z = coords[2];

    Eigen::Vector3d displacement;
    displacement << 0.01, 0.02, 0.03;
    h5_particle.displacement_x = displacement[0];
    h5_particle.displacement_y = displacement[1];
    h5_particle.displacement_z = displacement[2];

    Eigen::Vector3d lsize;
    lsize << 0.25, 0.5, 0.75;
    h5_particle.nsize_x = lsize[0];
    h5_particle.nsize_y = lsize[1];
    h5_particle.nsize_z = lsize[2];

    Eigen::Vector3d velocity;
    velocity << 1.5, 2.5, 3.5;
    h5_particle.velocity_x = velocity[0];
    h5_particle.velocity_y = velocity[1];
    h5_particle.velocity_z = velocity[2];

    Eigen::Matrix<double, 6, 1> stress;
    stress << 11.5, -12.5, 13.5, 14.5, -15.5, 16.5;
    h5_particle.stress_xx = stress[0];
    h5_particle.stress_yy = stress[1];
    h5_particle.stress_zz = stress[2];
    h5_particle.tau_xy = stress[3];
    h5_particle.tau_yz = stress[4];
    h5_particle.tau_xz = stress[5];

    Eigen::Matrix<double, 6, 1> strain;
    strain << 0.115, -0.125, 0.135, 0.145, -0.155, 0.165;
    h5_particle.strain_xx = strain[0];
    h5_particle.strain_yy = strain[1];
    h5_particle.strain_zz = strain[2];
    h5_particle.gamma_xy = strain[3];
    h5_particle.gamma_yz = strain[4];
    h5_particle.gamma_xz = strain[5];

    h5_particle.epsilon_v = strain.head(Dim).sum();

    h5_particle.status = true;

    h5_particle.cell_id = 1;

    h5_particle.volume = 2.;

    h5_particle.material_id = 1;

    h5_particle.nstate_vars = 0;

    for (unsigned i = 0; i < h5_particle.nstate_vars; ++i)
      h5_particle.svars[i] = 0.;

    h5_particle.liquid_mass = 100.1;

    Eigen::Vector3d liquid_velocity;
    liquid_velocity << 5.5, 2.1, 4.2;
    h5_particle.liquid_velocity_x = liquid_velocity[0];
    h5_particle.liquid_velocity_y = liquid_velocity[1];
    h5_particle.liquid_velocity_z = liquid_velocity[2];

    h5_particle.porosity = 0.33;

    h5_particle.liquid_saturation = 1.;

    h5_particle.liquid_material_id = 2;

    h5_particle.nliquid_state_vars = 1;

    for (unsigned i = 0; i < h5_particle.nliquid_state_vars; ++i)
      h5_particle.liquid_svars[i] = 0.;

    // Check send and receive particle with HDF5
    SECTION("Check send and receive particle with HDF5") {
      // Get number of MPI ranks
      int mpi_size;
      MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

      // If on same rank
      int sender = 0;
      int receiver = 0;
      // Get rank and do the corresponding job for mpi size == 2
      if (mpi_size == 2) receiver = 1;

      int mpi_rank;
      MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

      // Send particle
      if (mpi_rank == sender) {
        // Initialize MPI datatypes
        MPI_Datatype particle_type =
            mpm::register_mpi_particle_type(h5_particle);
        MPI_Send(&h5_particle, 1, particle_type, receiver, 0, MPI_COMM_WORLD);
        mpm::deregister_mpi_particle_type(particle_type);
      }

      // Receive particle
      if (mpi_rank == receiver) {
        // Receive the messid
        struct mpm::HDF5ParticleTwoPhase received;
        MPI_Datatype particle_type = mpm::register_mpi_particle_type(received);
        MPI_Recv(&received, 1, particle_type, sender, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        mpm::deregister_mpi_particle_type(particle_type);

        REQUIRE(h5_particle.id == received.id);
        REQUIRE(h5_particle.mass == received.mass);
        REQUIRE(h5_particle.pressure ==
                Approx(received.pressure).epsilon(Tolerance));
        REQUIRE(h5_particle.volume ==
                Approx(received.volume).epsilon(Tolerance));

        REQUIRE(h5_particle.coord_x ==
                Approx(received.coord_x).epsilon(Tolerance));
        REQUIRE(h5_particle.coord_y ==
                Approx(received.coord_y).epsilon(Tolerance));
        REQUIRE(h5_particle.coord_z ==
                Approx(received.coord_z).epsilon(Tolerance));

        REQUIRE(h5_particle.displacement_x ==
                Approx(received.displacement_x).epsilon(Tolerance));
        REQUIRE(h5_particle.displacement_y ==
                Approx(received.displacement_y).epsilon(Tolerance));
        REQUIRE(h5_particle.displacement_z ==
                Approx(received.displacement_z).epsilon(Tolerance));

        REQUIRE(h5_particle.nsize_x == received.nsize_x);
        REQUIRE(h5_particle.nsize_y == received.nsize_y);
        REQUIRE(h5_particle.nsize_z == received.nsize_z);

        REQUIRE(h5_particle.velocity_x ==
                Approx(received.velocity_x).epsilon(Tolerance));
        REQUIRE(h5_particle.velocity_y ==
                Approx(received.velocity_y).epsilon(Tolerance));
        REQUIRE(h5_particle.velocity_z ==
                Approx(received.velocity_z).epsilon(Tolerance));

        REQUIRE(h5_particle.stress_xx ==
                Approx(received.stress_xx).epsilon(Tolerance));
        REQUIRE(h5_particle.stress_yy ==
                Approx(received.stress_yy).epsilon(Tolerance));
        REQUIRE(h5_particle.stress_zz ==
                Approx(received.stress_zz).epsilon(Tolerance));
        REQUIRE(h5_particle.tau_xy ==
                Approx(received.tau_xy).epsilon(Tolerance));
        REQUIRE(h5_particle.tau_yz ==
                Approx(received.tau_yz).epsilon(Tolerance));
        REQUIRE(h5_particle.tau_xz ==
                Approx(received.tau_xz).epsilon(Tolerance));

        REQUIRE(h5_particle.strain_xx ==
                Approx(received.strain_xx).epsilon(Tolerance));
        REQUIRE(h5_particle.strain_yy ==
                Approx(received.strain_yy).epsilon(Tolerance));
        REQUIRE(h5_particle.strain_zz ==
                Approx(received.strain_zz).epsilon(Tolerance));
        REQUIRE(h5_particle.gamma_xy ==
                Approx(received.gamma_xy).epsilon(Tolerance));
        REQUIRE(h5_particle.gamma_yz ==
                Approx(received.gamma_yz).epsilon(Tolerance));
        REQUIRE(h5_particle.gamma_xz ==
                Approx(received.gamma_xz).epsilon(Tolerance));

        REQUIRE(h5_particle.epsilon_v ==
                Approx(received.epsilon_v).epsilon(Tolerance));
        REQUIRE(h5_particle.status == received.status);

        REQUIRE(h5_particle.cell_id == received.cell_id);
        REQUIRE(h5_particle.material_id == received.cell_id);

        REQUIRE(h5_particle.nstate_vars == received.nstate_vars);
        for (unsigned i = 0; i < h5_particle.nstate_vars; ++i)
          REQUIRE(h5_particle.svars[i] ==
                  Approx(received.svars[i]).epsilon(Tolerance));

        REQUIRE(h5_particle.liquid_mass ==
                Approx(received.liquid_mass).epsilon(Tolerance));
        REQUIRE(h5_particle.liquid_velocity_x ==
                Approx(received.liquid_velocity_x).epsilon(Tolerance));
        REQUIRE(h5_particle.liquid_velocity_y ==
                Approx(received.liquid_velocity_y).epsilon(Tolerance));
        REQUIRE(h5_particle.liquid_velocity_z ==
                Approx(received.liquid_velocity_z).epsilon(Tolerance));
        REQUIRE(h5_particle.porosity ==
                Approx(received.porosity).epsilon(Tolerance));
        REQUIRE(h5_particle.liquid_saturation ==
                Approx(received.liquid_saturation).epsilon(Tolerance));
        REQUIRE(h5_particle.liquid_material_id ==
                Approx(received.liquid_material_id).epsilon(Tolerance));

        REQUIRE(h5_particle.nliquid_state_vars == received.nliquid_state_vars);
        for (unsigned i = 0; i < h5_particle.nliquid_state_vars; ++i)
          REQUIRE(h5_particle.liquid_svars[i] ==
                  Approx(received.liquid_svars[i]).epsilon(Tolerance));
      }
    }

    // Check initialise particle from HDF5 file
    SECTION("Check initialise particle with HDF5 across MPI processes") {
      // Get number of MPI ranks
      int mpi_size;
      MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

      int mpi_rank;
      MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

      // If on same rank
      int sender = 0;
      int receiver = 0;

      // Get rank and do the corresponding job for mpi size == 2
      if (mpi_size == 2) receiver = 1;

      // Initial particle coordinates
      Eigen::Matrix<double, 3, 1> pcoordinates;
      pcoordinates.setZero();

      if (mpi_rank == sender) {
        // Create and initialzie particle with HDF5 data
        std::shared_ptr<mpm::ParticleBase<Dim>> particle =
            std::make_shared<mpm::TwoPhaseParticle<Dim>>(id, pcoordinates);

        // Assign material
        unsigned solid_mid = 1;
        unsigned liquid_mid = 2;
        // Initialise material
        Json jsolid_material;
        Json jliquid_material;
        jsolid_material["density"] = 1000.;
        jsolid_material["youngs_modulus"] = 1.0E+7;
        jsolid_material["poisson_ratio"] = 0.3;
        jsolid_material["porosity"] = 0.3;
        jsolid_material["k_x"] = 0.001;
        jsolid_material["k_y"] = 0.001;
        jsolid_material["k_z"] = 0.001;
        jliquid_material["density"] = 1000.;
        jliquid_material["bulk_modulus"] = 2.0E9;
        jliquid_material["dynamic_viscosity"] = 8.90E-4;

        auto solid_material =
            Factory<mpm::Material<Dim>, unsigned, const Json&>::instance()
                ->create("LinearElastic3D", std::move(solid_mid),
                         jsolid_material);
        auto liquid_material =
            Factory<mpm::Material<Dim>, unsigned, const Json&>::instance()
                ->create("Newtonian3D", std::move(liquid_mid),
                         jliquid_material);
        std::vector<std::shared_ptr<mpm::Material<Dim>>> materials;
        materials.emplace_back(solid_material);
        materials.emplace_back(liquid_material);

        // Reinitialise particle from HDF5 data
        REQUIRE(particle->initialise_particle(h5_particle, materials) == true);

        // Check particle id
        REQUIRE(particle->id() == h5_particle.id);
        // Check particle mass
        REQUIRE(particle->mass() == h5_particle.mass);
        // Check particle volume
        REQUIRE(particle->volume() == h5_particle.volume);
        // Check particle mass density
        REQUIRE(particle->mass_density() ==
                h5_particle.mass / h5_particle.volume);
        // Check particle status
        REQUIRE(particle->status() == h5_particle.status);

        // Check for coordinates
        auto coordinates = particle->coordinates();
        REQUIRE(coordinates.size() == Dim);
        for (unsigned i = 0; i < coordinates.size(); ++i)
          REQUIRE(coordinates(i) == Approx(coords(i)).epsilon(Tolerance));
        REQUIRE(coordinates.size() == Dim);

        // Check for displacement
        auto pdisplacement = particle->displacement();
        REQUIRE(pdisplacement.size() == Dim);
        for (unsigned i = 0; i < Dim; ++i)
          REQUIRE(pdisplacement(i) ==
                  Approx(displacement(i)).epsilon(Tolerance));

        // Check for size
        auto size = particle->natural_size();
        REQUIRE(size.size() == Dim);
        for (unsigned i = 0; i < size.size(); ++i)
          REQUIRE(size(i) == Approx(lsize(i)).epsilon(Tolerance));

        // Check velocity
        auto pvelocity = particle->velocity();
        REQUIRE(pvelocity.size() == Dim);
        for (unsigned i = 0; i < Dim; ++i)
          REQUIRE(pvelocity(i) == Approx(velocity(i)).epsilon(Tolerance));

        // Check stress
        auto pstress = particle->stress();
        REQUIRE(pstress.size() == stress.size());
        for (unsigned i = 0; i < stress.size(); ++i)
          REQUIRE(pstress(i) == Approx(stress(i)).epsilon(Tolerance));

        // Check strain
        auto pstrain = particle->strain();
        REQUIRE(pstrain.size() == strain.size());
        for (unsigned i = 0; i < strain.size(); ++i)
          REQUIRE(pstrain(i) == Approx(strain(i)).epsilon(Tolerance));

        // Check particle volumetric strain centroid
        REQUIRE(particle->volumetric_strain_centroid() ==
                h5_particle.epsilon_v);

        // Check cell id
        REQUIRE(particle->cell_id() == h5_particle.cell_id);

        // Check material id
        REQUIRE(particle->material_id() == h5_particle.material_id);

        // Check liquid mass
        REQUIRE(particle->liquid_mass() == h5_particle.liquid_mass);

        // Check liquid velocity
        auto pliquid_velocity = particle->liquid_velocity();
        REQUIRE(pliquid_velocity.size() == Dim);
        for (unsigned i = 0; i < Dim; ++i)
          REQUIRE(pliquid_velocity(i) ==
                  Approx(liquid_velocity(i)).epsilon(Tolerance));

        // Check porosity
        REQUIRE(particle->porosity() == h5_particle.porosity);

        // Check liquid material id
        REQUIRE(particle->material_id(mpm::ParticlePhase::Liquid) ==
                h5_particle.liquid_material_id);

        // Write Particle HDF5 data
        auto h5_twophase_send_ptr =
            std::static_pointer_cast<mpm::HDF5ParticleTwoPhase>(
                particle->hdf5_ptr());

        // Send MPI particle
        MPI_Datatype particle_type =
            mpm::register_mpi_particle_type(*h5_twophase_send_ptr);
        MPI_Send(&(*h5_twophase_send_ptr), 1, particle_type, receiver, 0,
                 MPI_COMM_WORLD);
        mpm::deregister_mpi_particle_type(particle_type);
      }
      if (mpi_rank == receiver) {
        // Receive the messid
        struct mpm::HDF5ParticleTwoPhase received;
        MPI_Datatype particle_type = mpm::register_mpi_particle_type(received);
        MPI_Recv(&received, 1, particle_type, sender, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        mpm::deregister_mpi_particle_type(particle_type);

        // Received particle
        std::shared_ptr<mpm::ParticleBase<Dim>> rparticle =
            std::make_shared<mpm::TwoPhaseParticle<Dim>>(id, pcoordinates);

        // Assign material
        unsigned solid_mid = 1;
        unsigned liquid_mid = 2;
        // Initialise material
        Json jsolid_material;
        Json jliquid_material;
        jsolid_material["density"] = 1000.;
        jsolid_material["youngs_modulus"] = 1.0E+7;
        jsolid_material["poisson_ratio"] = 0.3;
        jsolid_material["porosity"] = 0.3;
        jsolid_material["k_x"] = 0.001;
        jsolid_material["k_y"] = 0.001;
        jsolid_material["k_z"] = 0.001;
        jliquid_material["density"] = 1000.;
        jliquid_material["bulk_modulus"] = 2.0E9;
        jliquid_material["dynamic_viscosity"] = 8.90E-4;

        auto solid_material =
            Factory<mpm::Material<Dim>, unsigned, const Json&>::instance()
                ->create("LinearElastic3D", std::move(solid_mid),
                         jsolid_material);
        auto liquid_material =
            Factory<mpm::Material<Dim>, unsigned, const Json&>::instance()
                ->create("Newtonian3D", std::move(liquid_mid),
                         jliquid_material);
        std::vector<std::shared_ptr<mpm::Material<Dim>>> materials;
        materials.emplace_back(solid_material);
        materials.emplace_back(liquid_material);

        // Reinitialise particle from HDF5 data
        REQUIRE(rparticle->initialise_particle(received, materials) == true);

        // Check particle id
        REQUIRE(rparticle->id() == h5_particle.id);
        // Check particle mass
        REQUIRE(rparticle->mass() == h5_particle.mass);
        // Check particle volume
        REQUIRE(rparticle->volume() == h5_particle.volume);
        // Check particle mass density
        REQUIRE(rparticle->mass_density() ==
                h5_particle.mass / h5_particle.volume);
        // Check particle status
        REQUIRE(rparticle->status() == h5_particle.status);

        // Check for coordinates
        auto coordinates = rparticle->coordinates();
        REQUIRE(coordinates.size() == Dim);
        for (unsigned i = 0; i < coordinates.size(); ++i)
          REQUIRE(coordinates(i) == Approx(coords(i)).epsilon(Tolerance));
        REQUIRE(coordinates.size() == Dim);

        // Check for displacement
        auto pdisplacement = rparticle->displacement();
        REQUIRE(pdisplacement.size() == Dim);
        for (unsigned i = 0; i < Dim; ++i)
          REQUIRE(pdisplacement(i) ==
                  Approx(displacement(i)).epsilon(Tolerance));

        // Check for size
        auto size = rparticle->natural_size();
        REQUIRE(size.size() == Dim);
        for (unsigned i = 0; i < size.size(); ++i)
          REQUIRE(size(i) == Approx(lsize(i)).epsilon(Tolerance));

        // Check velocity
        auto pvelocity = rparticle->velocity();
        REQUIRE(pvelocity.size() == Dim);
        for (unsigned i = 0; i < Dim; ++i)
          REQUIRE(pvelocity(i) == Approx(velocity(i)).epsilon(Tolerance));

        // Check stress
        auto pstress = rparticle->stress();
        REQUIRE(pstress.size() == stress.size());
        for (unsigned i = 0; i < stress.size(); ++i)
          REQUIRE(pstress(i) == Approx(stress(i)).epsilon(Tolerance));

        // Check strain
        auto pstrain = rparticle->strain();
        REQUIRE(pstrain.size() == strain.size());
        for (unsigned i = 0; i < strain.size(); ++i)
          REQUIRE(pstrain(i) == Approx(strain(i)).epsilon(Tolerance));

        // Check particle volumetric strain centroid
        REQUIRE(rparticle->volumetric_strain_centroid() ==
                h5_particle.epsilon_v);

        // Check cell id
        REQUIRE(rparticle->cell_id() == h5_particle.cell_id);

        // Check material id
        REQUIRE(rparticle->material_id() == h5_particle.material_id);

        // Get Particle HDF5 data
        auto h5_twophase_received =
            std::static_pointer_cast<mpm::HDF5ParticleTwoPhase>(
                rparticle->hdf5_ptr());

        // State variables
        REQUIRE(h5_twophase_received->nstate_vars == h5_particle.nstate_vars);
        // State variables
        for (unsigned i = 0; i < h5_particle.nstate_vars; ++i)
          REQUIRE(h5_twophase_received->svars[i] ==
                  Approx(h5_particle.svars[i]).epsilon(Tolerance));

        // Check liquid mass
        REQUIRE(rparticle->liquid_mass() == h5_particle.liquid_mass);

        // Check liquid velocity
        auto pliquid_velocity = rparticle->liquid_velocity();
        REQUIRE(pliquid_velocity.size() == Dim);
        for (unsigned i = 0; i < Dim; ++i)
          REQUIRE(pliquid_velocity(i) ==
                  Approx(liquid_velocity(i)).epsilon(Tolerance));

        // Check porosity
        REQUIRE(rparticle->porosity() == h5_particle.porosity);

        // Check liquid material id
        REQUIRE(rparticle->material_id(mpm::ParticlePhase::Liquid) ==
                h5_particle.liquid_material_id);

        // Liquid state variables
        REQUIRE(h5_twophase_received->nliquid_state_vars ==
                h5_particle.nliquid_state_vars);
        // Liquid state variables
        for (unsigned i = 0; i < h5_particle.nliquid_state_vars; ++i)
          REQUIRE(h5_twophase_received->liquid_svars[i] ==
                  Approx(h5_particle.liquid_svars[i]).epsilon(Tolerance));
      }
    }
  }
}
#endif  // MPI
