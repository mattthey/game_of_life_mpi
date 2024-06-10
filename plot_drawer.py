import matplotlib.pyplot as plt
import subprocess

processes = [1, 2, 4, 8]
times = []


def draw_plot():
    speedup = [times[0] / t for t in times]
    efficiency = [s / p for s, p in zip(speedup, processes)]

    # Plotting
    plt.figure()
    plt.subplot(1, 2, 1)
    plt.plot(processes, speedup, marker='o')
    plt.xlabel('Number of Processes')
    plt.ylabel('Speedup')
    plt.title('Speedup vs. Number of Processes')

    plt.subplot(1, 2, 2)
    plt.plot(processes, efficiency, marker='o')
    plt.xlabel('Number of Processes')
    plt.ylabel('Efficiency')
    plt.title('Efficiency vs. Number of Processes')

    plt.tight_layout()
    # plt.savefig('result.png')
    plt.show()


def fill_times():
    for p in processes:
        output = subprocess.run(['mpirun', '-n', str(p), './build/game_of_life_mpi'],
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE,
                                text=True)
        print(output.stdout)
        lines = output.stdout.strip().split('\n')
        print('--------')
        print(lines)
        print('--------')
        times.append(float(lines[-1].split(' ')[-2]))


fill_times()
draw_plot()