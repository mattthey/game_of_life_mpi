import matplotlib.pyplot as plt
import subprocess

processes = [1.0, 2.0, 4.0, 8.0]
times = []


def draw_plot():
    speedup = [times[0] / t for t in times]

    # Plotting
    plt.figure()
    plt.subplot(1, 2, 1)
    plt.plot(processes, speedup, marker='o')
    plt.xlabel('Number of Processes')
    plt.ylabel('Speedup')
    plt.title('Speedup vs. Number of Processes')

    plt.subplot(1, 2, 2)
    plt.plot(processes, times, marker='o')
    plt.xlabel('Number of Processes')
    plt.ylabel('Time')
    plt.title('Time vs. Number of Processes')

    plt.tight_layout()
    # plt.savefig('result.png')
    plt.show()


def fill_times():
    for p in processes:
        output = subprocess.run(['mpirun', '-n', str(int(p)), './build/game_of_life_mpi'],
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE,
                                text=True)
        print(output.stderr)
        print(output.stdout)
        summ = 0
        count = 0
        for line in output.stdout.strip().split('\n'):
            if 'Elapsed time:' in line:
                summ += float(line.split(' ')[-4])
                count += 1
        
        times.append((summ / count))


fill_times()
draw_plot()