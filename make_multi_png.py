import glob
import math
import matplotlib.pyplot as plt
import matplotlib.image as mpimg

def plot_images_in_grid(output_filename):
    # Get all png files in the ./png directory
    image_files = glob.glob('./png/*.png')
    
    # Calculate the total number of images
    n = len(image_files)
    
    if n == 0:
        print("No images found in the directory.")
        return
    
    # Determine the number of rows and columns
    b = math.ceil(math.sqrt(n))
    a = math.ceil(n / b)

    # Create a figure with specified number of subplots
    fig, axes = plt.subplots(a, b, figsize=(b * 2, a * 2))
    
    # Flatten the axes array for easy iteration if necessary
    axes = axes.flatten()
    
    # Plot each image in the grid
    for i, image_file in enumerate(image_files):
        # Read and plot the image
        img = mpimg.imread(image_file)
        axes[i].imshow(img)
        axes[i].axis('off')  # Hide the axes

    # Hide any unused subplots
    for j in range(i + 1, a * b):
        axes[j].axis('off')

    # Adjust layout and save the figure
    plt.tight_layout()
    plt.savefig(output_filename)
    plt.close()

if __name__ == "__main__":
    plot_images_in_grid('multi_graph.png')