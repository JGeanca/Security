def distribute_indices(num_indices, num_computers):
    indices_per_computer = num_indices // num_computers
    extra_indices = num_indices % num_computers
    indices = []
    start_index = 0
    for i in range(num_computers):
        end_index = start_index + indices_per_computer
        if i < extra_indices:
            end_index += 1
        indices.append((start_index, end_index))
        start_index = end_index
    return indices

if __name__ == "__main__":
    num_indices = 100000
    num_computers = int(input("Ingrese la cantidad de computadoras disponibles: "))

    if num_computers <= 0:
        print("El número de computadoras debe ser mayor que 0.")
    else:
        indices = distribute_indices(num_indices, num_computers)
        print("Índices repartidos para cada computadora:")
        for i, idx_range in enumerate(indices):
            print(f"Computadora {i + 1}: Índices {idx_range[0]} - {idx_range[1] - 1}")
