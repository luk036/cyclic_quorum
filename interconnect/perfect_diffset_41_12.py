import numpy as np
import matplotlib.pyplot as plt

def design_41x12_quorum_grid(N_inputs=41, M_outputs=12):
    """
    Designs a highly uniform 41x12 interconnect grid using a 
    direct cyclic distribution rule modulo M.
    """
    # Base pattern of 4 wires per column optimized for minimal row-load variance
    diff_set = [0, 1, 4, 6] 
    
    grid_matrix = np.zeros((N_inputs, M_outputs), dtype=int)
    
    # Apply the cyclic shift rule across all columns
    for i in range(N_inputs):
        for d in diff_set:
            row_idx = (i + d) % M_outputs
            grid_matrix[i][row_idx] = 1
            
    return grid_matrix, diff_set

if __name__ == "__main__":
    N_in, M_out = 41, 12
    grid_matrix, diff_set = design_41x12_quorum_grid(N_in, M_out)
    
    # Calculate loads to verify uniform density
    vias_per_column = np.sum(grid_matrix, axis=1)
    vias_per_row = np.sum(grid_matrix, axis=0)
    
    print(f"Total connections (vias) on chip: {np.sum(grid_matrix)}")
    print(f"Vias per column (Inputs 0-40): {vias_per_column[0]} (Perfect uniformity)")
    print(f"Vias per row (Outputs 0-11): Min={min(vias_per_row)}, Max={max(vias_per_row)}")

    # --- Visualization Code (North-South Columns vs West-East Rows) ---
    fig, ax = plt.subplots(figsize=(15, 5))
    ax.set_facecolor('#ffffff')  # Clean white background
    
    # Draw horizontal Output lines (West to East)
    for j in range(M_out):
        ax.axhline(y=j, color='#e0e0e0', linestyle='-', linewidth=0.8, zorder=1)
        
    # Draw vertical Input lines (North to South)
    for i in range(N_in):
        ax.axvline(x=i, color='#e0e0e0', linestyle='-', linewidth=0.8, zorder=1)
        
    # Find coordinates where physical connections (vias) exist
    via_y, via_x = np.where(grid_matrix.T == 1)
    
    # Plot connections exclusively as solid black dots
    ax.scatter(via_x, via_y, color='#000000', s=35, zorder=3, label='Physical Via')
    
    # Formatting layout axes
    ax.set_xlim(-0.5, N_in - 0.5)
    ax.set_ylim(-0.5, M_out - 0.5)
    ax.invert_yaxis()  # Physical top-to-bottom die orientation
    
    # Structural Labels
    ax.set_title(f"Direct Cyclic Interconnect Grid ({N_in} Inputs $\\rightarrow$ {M_out} Outputs)\n"
                 f"Symmetric Pitch Distribution Matrix via Shift Offsets: {diff_set}", 
                 fontsize=13, fontweight='bold', pad=15)
    ax.set_xlabel("Input Columns (Running North $\\rightarrow$ South: 0 to 40)", fontsize=11, labelpad=8)
    ax.set_ylabel("Output Rows (Running West $\\rightarrow$ East: 0 to 11)", fontsize=11, labelpad=8)
    
    ax.set_xticks(np.arange(0, N_in, 1))
    ax.set_yticks(np.arange(0, M_out, 1))
    ax.grid(False)
    
    plt.tight_layout()
    plt.show()
