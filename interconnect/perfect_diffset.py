import numpy as np
import matplotlib.pyplot as plt

def design_projective_quorum_grid():
    """
    Designs a highly routable, low-cost hierarchical interconnection 
    network using a (13, 4, 1) cyclic projective plane core.
    """
    # Core parameters based on a Perfect Difference Set (PDS)
    N_core = 13
    diff_set = [0, 1, 3, 9]  # Singer Difference Set modulo 13
    
    # Generate the sparse 13x13 cyclic quorum core matrix
    # grid[i][j] = 1 represents a physical black dot (via) at intersection
    core_grid = np.zeros((N_core, N_core), dtype=int)
    for i in range(N_core):
        for j in range(N_core):
            if (i - j) % N_core in diff_set:
                core_grid[i][j] = 1
                
    return core_grid, diff_set

if __name__ == "__main__":
    core_grid, diff_set = design_projective_quorum_grid()
    N_core = core_grid.shape[0]
    
    # --- Visualization Code (North-South Columns vs West-East Rows) ---
    fig, ax = plt.subplots(figsize=(8, 8))
    ax.set_facecolor('#ffffff')  # Clean white background for clear contrast
    
    # Draw the physical interconnect wire lines
    for j in range(N_core):
        ax.axhline(y=j, color='#e0e0e0', linestyle='-', linewidth=1, zorder=1)
    for i in range(N_core):
        ax.axvline(x=i, color='#e0e0e0', linestyle='-', linewidth=1, zorder=1)
        
    # Find coordinates where physical connections (vias) exist
    via_y, via_x = np.where(core_grid.T == 1)
    
    # Plot connections strictly as black dots
    ax.scatter(via_x, via_y, color='#000000', s=60, zorder=3, label='Physical Via')
    
    # Grid formatting
    ax.set_xlim(-0.5, N_core - 0.5)
    ax.set_ylim(-0.5, N_core - 0.5)
    ax.invert_yaxis()  # Standard physical chip layout orientation (top-to-bottom)
    
    # Title and Labels matching the structural alignment
    ax.set_title(f"Hierarchical Quorum Interconnect Core Grid (13x13)\n"
                 f"Perfect Difference Set: {diff_set} (Any-to-Any Intersection $\\lambda=1$)", 
                 fontsize=12, fontweight='bold', pad=15)
    ax.set_xlabel("Virtual Core Inputs (Columns: North $\\rightarrow$ South)", fontsize=10, labelpad=8)
    ax.set_ylabel("Virtual Core Outputs (Rows: West $\\rightarrow$ East)", fontsize=10, labelpad=8)
    
    ax.set_xticks(np.arange(0, N_core, 1))
    ax.set_yticks(np.arange(0, N_core, 1))
    ax.grid(False)
    
    plt.tight_layout()
    plt.show()
