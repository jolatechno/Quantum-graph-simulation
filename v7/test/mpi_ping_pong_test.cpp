#include <ranges>

#include "../IQS/src/iqs_mpi.hpp"
#include "../IQS/src/rules/qcgd.hpp"

#include <unistd.h>

void mid_step_function(iqs::mpi::mpi_it_t const &state, iqs::mpi::mpi_it_t const &buffer, iqs::mpi::mpi_sy_it_t const &sy_it, MPI_Comm comunicator) {
	int size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Barrier(MPI_COMM_WORLD);

	size_t previous_num_graphs = buffer.get_total_num_object(comunicator);
	size_t symbolic_num_graphs = sy_it.get_total_num_object(comunicator);
	size_t num_graphs_after_interferences = sy_it.get_total_num_object_after_interferences(comunicator);
	size_t current_num_graphs = state.get_total_num_object(comunicator);

	if (rank == 0) {
		usleep(1000);
		std::cout << "\n";
		std::cout << previous_num_graphs << "->(" << symbolic_num_graphs << "->" << num_graphs_after_interferences << ")->" << current_num_graphs << "\n";
	}

	for (int i = 0; i < size; ++i) {
		usleep(1000);
		MPI_Barrier(MPI_COMM_WORLD);
		if (rank == i)
			std::cout << "    " << state.num_object << " for rank " << rank << "/" << size << "\n";
	}
	MPI_Barrier(MPI_COMM_WORLD);
	usleep(1000);
}

int main(int argc, char* argv[]) {
	int size, rank, provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
    if(provided < MPI_THREAD_SERIALIZED) {
        printf("The threading support level is lesser than that demanded.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	iqs::rules::qcgd::utils::max_print_num_graphs = 10;

	iqs::mpi::mpi_it_t state, buffer;
	iqs::mpi::mpi_sy_it_t sy_it;

	iqs::tolerance = 1e-8;

	auto [n_iter, reversed_n_iter, local_state, rules, max_num_object] = iqs::rules::qcgd::flags::parse_simulation(argv[1]);

	iqs::rules::qcgd::utils::print(state);

	if (rank == 0)
		for (int i = 0; i < local_state.num_object; ++i) {
			std::complex<PROBA_TYPE> mag;
			size_t size;
			char const *object_begin;
			local_state.get_object(i, object_begin, size, mag);
			state.append(object_begin, object_begin + size, mag);
		}

	if (rank == 0) {
		std::cout << "initial state:\n";
		iqs::rules::qcgd::utils::print(state);
	}

	for (int i = 0; i < n_iter; ++i) {
		for (auto [n_iter, is_rule, modifier, rule, _, __] : rules)
			for (int j = 0; j < n_iter; ++j)
				if (is_rule) {
					iqs::mpi::simulate(state, rule, buffer, sy_it, MPI_COMM_WORLD, max_num_object);
				} else
					iqs::simulate(state, modifier);
		mid_step_function(state, buffer, sy_it, MPI_COMM_WORLD);
	}
	for (int i = 0; i < reversed_n_iter; ++i) {
		if (i == reversed_n_iter - 1)
			iqs::collision_tolerance = 0;
		
		for (auto [n_iter, is_rule, _, __, modifier, rule] : rules | std::views::reverse)
			for (int j = 0; j < n_iter; ++j)
				if (is_rule) {
					iqs::mpi::simulate(state, rule, buffer, sy_it, MPI_COMM_WORLD, max_num_object);
				} else
					iqs::simulate(state, modifier);

		mid_step_function(state, buffer, sy_it, MPI_COMM_WORLD);
	}

	state.gather_objects(MPI_COMM_WORLD);
	
	if (rank == 0) {
		std::cout << "\nfinal state:\n";
		iqs::rules::qcgd::utils::print(state);
	}

	MPI_Finalize();
	return 0;
}
