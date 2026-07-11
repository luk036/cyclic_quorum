import numpy as np
import matplotlib.pyplot as plt

def design_tiled_quorum_grid(num_tiles=3):
    """
    Designs a 39x13 interconnection network by horizontally tiling
    a highly routable (13, 4, 1) cyclic projective plane core.
    """
    N_core = 13
    diff_set = [0, 1, 3, 9]  # Singer Difference Set modulo 13
    
    # 1. Generate the base 13x13 sparse core matrix
    base_core = np.zeros((N_core, N_core), dtype=int)
    for i in range(N_core):
        for j in range(N_core):
            if (i - j) % N_core in diff_set:
                base_core[i][j] = 1
                
    # 2. Tile the core horizontally to create the expanded input space
    # For num_tiles=3, this transitions the grid from 13x13 to 39x13
    tiled_grid = np.tile(base_core, (num_tiles, 1))
    
    return tiled_grid, diff_set

if __name__ == "__main__":
    num_tiles = 3
    tiled_grid, diff_set = design_tiled_quorum_grid(num_tiles=num_tiles)
    
    # Grid dimensions
    N_inputs = tiled_grid.shape[0]   # 39 Columns
    M_outputs = tiled_grid.shape[1]  # 13 Rows
    
    # --- Visualization Code (North-South Columns vs West-East Rows) ---
    fig, ax = plt.subplots(figsize=(15, 6))
    ax.set_facecolor('#ffffff')  # Crisp white background for maximum via contrast
    
    # Draw horizontal Output lines (West to East)
    for j in range(M_outputs):
        ax.axhline(y=j, color='#e0e0e0', linestyle='-', linewidth=0.8, zorder=1)
        
    # Draw vertical Input lines (North to South)
    for i in range(N_inputs):
        ax.axvline(x=i, color='#e0e0e0', linestyle='-', linewidth=0.8, zorder=1)
        
    # Highlight the boundary lines between the tiled blocks to show the modular structure
    for t in range(1, num_tiles):
        ax.axvline(x=t * 13 - 0.5, color='#ff3366', linestyle='--', linewidth=1.5, zorder=2, 
                   label='Tile Boundary' if t == 1 else "")
        
    # Find coordinates where physical connections (vias) exist
    via_y, via_x = np.where(tiled_grid.T == 1)
    
    # Plot connections cleanly as solid black dots
    ax.scatter(via_x, via_y, color='#000000', s=40, zorder=3, label='Physical Via')
    
    # Formatting layout axes
    ax.set_xlim(-0.5, N_inputs - 0.5)
    ax.set_ylim(-0.5, M_outputs - 0.5)
    ax.invert_yaxis()  # Physical top-to-bottom die reading orientation
    
    # Descriptive titles and structured structural labels
    ax.set_title(f"Tiled Quorum Interconnect Grid ({N_inputs} Inputs $\\rightarrow$ {M_outputs} Outputs)\n"
                 f"Constructed from {num_tiles} Horizontally Tiled (13, 4, 1) Projective Plane Matrices", 
                 fontsize=13, fontweight='bold', pad=15)
    ax.set_xlabel("Input Columns (Running North $\\rightarrow$ South: 0 to 38)", fontsize=11, labelpad=8)
    ax.set_ylabel("Output Rows (Running West $\\rightarrow$ East: 0 to 12)", fontsize=11, labelpad=8)
    
    ax.set_xticks(np.arange(0, N_inputs, 1))
    ax.set_yticks(np.arange(0, M_outputs, 1))
    ax.grid(False)
    
    plt.legend(loc='upper right')
    plt.tight_layout()
    plt.show()
