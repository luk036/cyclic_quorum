import numpy as np
import matplotlib.pyplot as plt
from itertools import combinations

def design_and_plot_grid_interconnect(N=100, M=23, fan_out=6):
    """
    Designs an optimal cyclic quorum-based interconnection network
    and visualizes it as a physical North-South / West-East layout grid
    with black dots representing connections.
    """
    # 1. Calculate the virtual home positions of the M outputs on the N-scale line
    P = [int(round(j * N / M)) % N for j in range(M)]
    
    # 2. Candidate pool of spatial offsets (wire spans)
    candidate_offsets = [0, 1, 2, 4, 5, 8, 10, 12, 15, 20, 25, 33, 40, 50, 75]
    candidate_offsets = [x for x in candidate_offsets if x < N]
    
    best_diff_set = None
    min_variance = float('inf')
    optimal_connections = {}

    # 3. Find the offset combination that optimizes wire/load distribution
    for diff_set in combinations(candidate_offsets, fan_out):
        diff_set = list(diff_set)
        output_counts = np.zeros(M)
        current_connections = {i: [] for i in range(N)}
        
        for i in range(N):
            for j in range(M):
                if (i - P[j]) % N in diff_set:
                    current_connections[i].append(j)
                    output_counts[j] += 1
                    
        variance = np.var(output_counts)
        if variance < min_variance:
            min_variance = variance
            best_diff_set = diff_set
            optimal_connections = current_connections

    # --- Matplotlib Grid Visualization Logic ---
    fig, ax = plt.subplots(figsize=(14, 8))
    ax.set_facecolor('#ffffff') # Clean white background to contrast black dots
    
    # Draw horizontal Output lines (West to East)
    for j in range(M):
        ax.axhline(y=j, color='#dcdde1', linestyle='-', linewidth=0.8, zorder=1)
        
    # Draw vertical Input lines (North to South)
    for i in range(N):
        ax.axvline(x=i, color='#dcdde1', linestyle='-', linewidth=0.8, zorder=1)
        
    # Plot the connections as black dots at intersection coordinates (i, j)
    connection_x = []
    connection_y = []
    
    for i in range(N):
        for j in optimal_connections[i]:
            connection_x.append(i)
            connection_y.append(j)
            
    ax.scatter(connection_x, connection_y, color='#000000', s=15, zorder=3, label='Connection (Via)')
    
    # Adjust axes limits and directions
    ax.set_xlim(-1, N)
    ax.set_ylim(-1, M)
    
    # Invert Y-axis so output index 0 starts at the top, matching a top-down physical die reading
    ax.invert_yaxis()
    
    # Labeling and layout adjustments
    ax.set_title(f"Crossbar Grid Routing Via Cyclic Quorums ({N} Inputs $\\rightarrow$ {M} Outputs)\n"
                 f"Optimal Difference Set: {best_diff_set}", fontsize=14, pad=15, fontweight='bold')
    
    ax.set_xlabel("Inputs (Running North $\\rightarrow$ South)", fontsize=12, labelpad=10)
    ax.set_ylabel("Outputs (Running West $\\rightarrow$ East)", fontsize=12, labelpad=10)
    
    # Set tick spacing for readability
    ax.set_xticks(np.arange(0, N+1, 10))
    ax.set_yticks(np.arange(0, M, 2))
    
    ax.grid(False)
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    # Execute the crossbar grid mapping and plot
    design_and_plot_interconnect = design_and_plot_grid_interconnect(N=100, M=23, fan_out=6)
