import pandas as pd
import matplotlib.pyplot as plt

# 1. Read the CSV data
# Ensure '2_1.csv' is in the same directory as this script
df = pd.read_csv("2_1.csv")

# 2. Clean column names (strips accidental spaces from the CSV headers)
df.columns = df.columns.str.strip()

# 3. Initialize the plot figure
plt.figure(figsize=(10, 6))

# 4. Plot the Custom Centralized Barrier data
plt.plot(df['processes'], df['custom_latency_ms'], 
         marker='o', 
         label='Custom Barrier (Centralized)', 
         color='#1f77b4', 
         linewidth=2)

# 5. Plot the Built-in MPI Barrier data
plt.plot(df['processes'], df['built_in_latency_ms'], 
         marker='s', 
         label='Built-In MPI_Barrier', 
         color='#ff7f0e', 
         linewidth=2, 
         linestyle='--')

# 6. Formatting and Labels
plt.title('MPI Barrier Latency vs. Number of Processes', fontsize=14, fontweight='bold')
plt.xlabel('Number of Processes', fontsize=12)
plt.ylabel('Average Latency (ms)', fontsize=12)

# Ensure the x-axis ticks exactly match the process numbers (2, 4, 6... 24)
plt.xticks(df['processes']) 

# Add a grid and a legend
plt.grid(True, linestyle='--', alpha=0.7)
plt.legend(fontsize=11)
plt.tight_layout()

# 7. Save the plot as an image file and display it
plt.savefig('barrier_latency_plot.png', dpi=300)
plt.show()