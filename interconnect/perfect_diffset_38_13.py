import numpy as np
import matplotlib.pyplot as plt

def design_truncated_quorum_grid(target_inputs=38, N_core=13):
    """
    Designs a 38x13 interconnection network by truncating a 
    3-tile horizontally expanded (13, 4, 1) projective plane fabric.
    """
    diff_set = [0, 1, 3, 9]  # Singer Difference Set modulo 13
    
    # 1. Generate the base 13x13 sparse core matrix
    base_core = np.zeros((N_core, N_core), dtype=int)
    for i in range(N_core):
        for j in range(N_core):
            if (i - j) % N_core in diff_set:
                base_core[i][j] = 1
                
    # 2. Tile the core horizontally 3 times to get 39 inputs
    full_tiled_grid = np.tile(base_core, (3, 1))
    
    # 3. Truncate the grid to exactly the target number of inputs (38 columns)
    # This slices off the final column (index 38), leaving columns 0 to 37
    truncated_grid = full_tiled_grid[:target_inputs, :]
    
    return truncated_grid, diff_set

if __name__ == "__main__":
    target_in = 38
    grid_matrix, diff_set = design_truncated_quorum_grid(target_inputs=target_in)
    
    N_inputs = grid_matrix.shape[0]   # 38 Columns
    M_outputs = grid_matrix.shape[1]  # 13 Rows
    
    # --- Visualization Code (North-South Columns vs West-East Rows) ---
    fig, ax = plt.subplots(figsize=(15, 6))
    ax.set_facecolor('#ffffff')  # Crisp white background
    
    # Draw horizontal Output lines (West to East)
    for j in range(M_outputs):
        ax.axhline(y=j, color='#e0e0e0', linestyle='-', linewidth=0.8, zorder=1)
        
    # Draw vertical Input lines (North to South)
    for i in range(N_inputs):
        ax.axvline(x=i, color='#e0e0e0', linestyle='-', linewidth=0.8, zorder=1)
        
    # Draw the internal macro tile boundaries (Tile 1 ends at 12.5, Tile 2 at 25.5)
    for t in [1, 2]:
        ax.axvline(x=t * 13 - 0.5, color='#ff3366', linestyle='--', linewidth=1.5, zorder=2, 
                   label='Core Tile Boundary' if t == 1 else "")
        
    # Locate and extract the physical connection coordinates
    via_y, via_x = np.where(grid_matrix.T == 1)
    
    # Plot connections exclusively as solid black dots
    ax.scatter(via_x, via_y, color='#000000', s=40, zorder=3, label='Physical Via')
    
    # Grid formatting boundaries
    ax.set_xlim(-0.5, N_inputs - 0.5)
    ax.set_ylim(-0.5, M_outputs - 0.5)
    ax.invert_yaxis()  # Physical top-down die orientation
    
    # Labeling
    ax.set_title(f"Truncated Tiled Quorum Interconnect Grid ({N_inputs} Inputs $\\rightarrow$ {M_outputs} Outputs)\n"
                 f"Constructed via a 39-Line Projective Core Fabric Truncated at Column 38", 
                 fontsize=13, fontweight='bold', pad=15)
    ax.set_xlabel("Input Columns (Running North $\\rightarrow$ South: 0 to 37)", fontsize=11, labelpad=8)
    ax.set_ylabel("Output Rows (Running West $\\rightarrow$ East: 0 to 12)", fontsize=11, labelpad=8)
    
    ax.set_xticks(np.arange(0, N_inputs, 1))
    ax.set_yticks(np.arange(0, M_outputs, 1))
    ax.grid(False)
    
    plt.legend(loc='upper right')
    plt.tight_layout()
    plt.show()
