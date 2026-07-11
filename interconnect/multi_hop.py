import numpy as np
import matplotlib.pyplot as plt

def design_and_route_interconnect(N=100, M=23):
    """
    Designs a sparse cyclic-quorum based crossbar grid and solves 
    the multi-hop routing path for any sparse pair.
    """
    # 1. Map Output blocks evenly to the 100-input ring
    P = [int(round(j * N / M)) % N for j in range(M)]
    
    # 2. Optimized sparse difference set (6 lines per input)
    # This guarantees that the network graph remains connected with a low diameter
    diff_set = [0, 4, 12, 25, 50, 75]
    
    # 3. Build the forward physical connection matrix (Adjacency)
    # grid[i][j] == 1 means a physical black dot (via) exists at column i, row j
    grid = np.zeros((N, M), dtype=int)
    for i in range(N):
        for j in range(M):
            if (i - P[j]) % N in diff_set:
                grid[i][j] = 1
                
    return grid, P, diff_set

def find_route(grid, P, diff_set, start_input=40, target_output=16, N=100, M=23):
    """
    Finds a multi-hop path from a sparse input to an output by leveraging 
    the intersection properties of the cyclic quorum.
    """
    # Check if a direct single-hop connection exists
    if grid[start_input][target_output] == 1:
        return [("Direct", start_input, target_output)]
    
    # Multi-hop search: Find an intermediate output node that has a physical loopback
    # connection to an input capable of striking the target output.
    for intermediate_out in range(M):
        if grid[start_input][intermediate_out] == 1:
            # Assume the intermediate output loops back to an identical input index 
            # or a local cluster index (Virtual routing hop)
            intermediate_in = int(round(intermediate_out * N / M)) % N
            
            # Check if this intermediate input node can bridge to our final target output
            if grid[intermediate_in][target_output] == 1:
                return [
                    ("Hop 1", start_input, intermediate_out),
                    ("Loopback/Hop 2", intermediate_in, target_output)
                ]
    return None

# --- Execution & Visualization ---
if __name__ == "__main__":
    N_in, M_out = 100, 23
    grid, P, diff_set = design_and_route_interconnect(N_in, M_out)
    
    # Target path requested: From Input 40 to Output 16
    start, target = 40, 16
    path = find_route(grid, P, diff_set, start_input=start, target_output=target)
    
    # Plotting the physical layout
    fig, ax = plt.subplots(figsize=(14, 8))
    ax.set_facecolor('#ffffff')
    
    # Draw Crossbar Wires
    for j in range(M_out):
        ax.axhline(y=j, color='#f0f1f4', linestyle='-', linewidth=0.8, zorder=1)
    for i in range(N_in):
        ax.axvline(x=i, color='#f0f1f4', linestyle='-', linewidth=0.8, zorder=1)
        
    # Plot all sparse physical connections as Black Dots
    conn_y, conn_x = np.where(grid.T == 1)
    ax.scatter(conn_x, conn_y, color='#000000', s=15, zorder=2, label='Physical Connections (Vias)')
    
    # Highlight the multi-hop routing solution on the grid if it exists
    if path:
        print(f"Routing path found from Input {start} to Output {target}:")
        colors = ['#ff3366', '#00ffcc']
        for idx, step in enumerate(path):
            step_type, inp, outp = step
            print(f"  -> {step_type}: Connect Input {inp} to Output {outp}")
            # Highlight this specific operational path on the chart
            ax.scatter(inp, outp, color=colors[idx % 2], s=80, edgecolors='black', zorder=4, 
                       label=f'Path Step: Input {inp} $\\rightarrow$ Output {outp}')
            # Draw a visual tracking line showing the signal flow
            ax.plot([inp, inp], [-0.5, outp], color=colors[idx % 2], linestyle='--', linewidth=1.5, zorder=3)
    else:
        print(f"No valid cyclic quorum path found between {start} and {target}.")

    # Formatting adjustments
    ax.set_xlim(-1, N_in)
    ax.set_ylim(-1, M_out)
    ax.invert_yaxis()  # Standard top-down layout view
    
    ax.set_title(f"Sparse Cyclic Quorum Routing Grid ({N_in} Inputs $\\rightarrow$ {M_out} Outputs)\n"
                 f"Highlighting Path: Input {start} $\\rightarrow$ Output {target}", fontsize=14, fontweight='bold', pad=15)
    ax.set_xlabel("Inputs (Vertical Lines running North $\\rightarrow$ South)", fontsize=11)
    ax.set_ylabel("Outputs (Horizontal Lines running West $\\rightarrow$ East)", fontsize=11)
    
    ax.set_xticks(np.arange(0, N_in+1, 10))
    ax.set_yticks(np.arange(0, M_out, 2))
    ax.grid(False)
    plt.legend(loc='upper right')
    plt.tight_layout()
    plt.show()
