import matplotlib.pyplot as plt

def plot_requests(w, x, y, z):

    plt.grid()
    plt.title("Desempenho dos servidores")
    plt.xlabel('Número de usuários concorrentes')
    plt.ylabel('Requisições respondidas (por segundo)')
    
    u = [1, 8, 32, 128, 255]
    plt.plot(u, w, color='b', label='Iterativo')
    plt.plot(u, x, color='y', label='Fork')
    plt.plot(u, y, color='r', label='Thread')
    plt.plot(u, z, color='g', label='Concorrente')
    plt.legend(loc='best')
    plt.show()


def main():

    w = [4373, 6351, 5009, 6208, 4964]
    x = [691, 1574, 1599, 1608, 1936]
    y = [1, 10, 82, 65, 87]
    z = [3047, 6616, 6392, 3838, 5354]

    plot_requests(w, x, y, z)


if __name__ == '__main__':
    main()
