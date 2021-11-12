#include "../IQS/src/iqs_mpi.hpp"
#include "../IQS/src/rules/qcgd.hpp"

#include <unistd.h>

void mid_step_function(iqs::mpi::mpi_it_t const &state, iqs::mpi::mpi_it_t const &buffer, iqs::mpi::mpi_sy_it_t const &sy_it, MPI_Comm comunicator, bool print=false) {
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
		if (!print)
			std::cout << previous_num_graphs << "->(" << symbolic_num_graphs << "->" << num_graphs_after_interferences << ")->" << current_num_graphs << "\n";
	}

	for (int i = 0; i < size; ++i) {
		usleep(1000);
		MPI_Barrier(MPI_COMM_WORLD);
		if (rank == i) {
			std::cout << (print ? "" : "    ") << state.num_object << " for rank " << rank << "/" << size << (print ? ":\n" : "\n");
			if (print) iqs::rules::qcgd::utils::print(state);
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
}

int main(int argc, char* argv[]) {
	int size, rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	iqs::rules::qcgd::utils::max_print_num_graphs = 4;

	iqs::mpi::mpi_it_t state, buffer;
	iqs::mpi::mpi_sy_it_t sy_it;

	auto [n_iter, reversed_n_iter, local_state, _, __] = iqs::rules::qcgd::flags::parse_simulation(argv[1]);

	iqs::rule_t *rule = new iqs::rules::qcgd::split_merge(0.25, 0.25);

	if (rank == 0)
		for (int i = 0; i < local_state.num_object; ++i) {
			std::complex<PROBA_TYPE> mag;
			size_t size;
			char const *object_begin = local_state.get_object(i, size, mag);
			state.append(object_begin, object_begin + size, mag);
		}

	mid_step_function(state, buffer, sy_it, MPI_COMM_WORLD, true);

	for (int i = 0; i < n_iter; ++i) {
		if (i == 1) state.distribute_objects(MPI_COMM_WORLD);

		iqs::simulate(state, iqs::rules::qcgd::step);
		iqs::mpi::simulate(state, rule, buffer, sy_it, MPI_COMM_WORLD);

		mid_step_function(state, buffer, sy_it, MPI_COMM_WORLD);
	}
	for (int i = 0; i < reversed_n_iter; ++i) {
		iqs::mpi::simulate(state, rule, buffer, sy_it, MPI_COMM_WORLD);
		iqs::simulate(state, iqs::rules::qcgd::reversed_step);

		mid_step_function(state, buffer, sy_it, MPI_COMM_WORLD);
	}

	state.gather_objects(MPI_COMM_WORLD);

	mid_step_function(state, buffer, sy_it, MPI_COMM_WORLD, true);

	MPI_Finalize();
	return 0;
}