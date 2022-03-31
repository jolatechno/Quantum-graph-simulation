#include "../QuIDS/src/iqds_mpi.hpp"
#include "../QuIDS/src/rules/qcgd.hpp"

int main(int argc, char* argv[]) {
	int provided, rank, size;
	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if(provided < MPI_THREAD_MULTIPLE) {
        printf("The threading support level is lesser than that demanded.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	iqds::mpi::mpi_it_t buffer1, buffer2;
	iqds::mpi::mpi_sy_it_t sy_it;
	iqds::it_t local_state;

	iqds::mpi::mpi_it_t *state = new iqds::mpi::mpi_it_t, *buffer = new iqds::mpi::mpi_it_t;

	auto [n_iter, reversed_n_iter, rules, max_allowed_num_object] = iqds::rules::qcgd::flags::parse_simulation(argv[1], local_state);

	if (rank == 0) {
		std::cout << "{\n\t\"command\" : \"" << argv[1] << "\",\n";
		std::cout << "\t\"iterations\" : [\n\t\t";

		iqds::rules::qcgd::utils::serialize(local_state, sy_it, 2);

		for (int i = 0; i < local_state.num_object; ++i) {
			std::complex<PROBA_TYPE> mag;
			size_t size;
			char const *object_begin;
			local_state.get_object(i, object_begin, size, mag);
			state->append(object_begin, object_begin + size, mag);
		}
	}

	for (int i = 0; i < n_iter; ++i) {
		for (auto [local_n_iter, is_rule, modifier, rule, _, __] : rules)
			for (int j = 0; j < local_n_iter; ++j)
				if (is_rule) {
					iqds::mpi::simulate(*state, rule, *buffer, sy_it, MPI_COMM_WORLD, max_allowed_num_object);

					std::swap(state, buffer);

					if (rank == 0) std::cout << ", ";
					iqds::rules::qcgd::utils::serialize(*state, sy_it, MPI_COMM_WORLD, 2);
				} else
					iqds::simulate(*state, modifier);
	}

	if (rank == 0) std::cout << "\n\t]\n}\n";

	MPI_Finalize();
}
