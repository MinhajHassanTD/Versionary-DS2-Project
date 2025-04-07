# Versionary User Guide

This document provides instructions for using the Versionary image-based version control system.

## Introduction

Versionary is an image-based version control system designed to efficiently manage and track changes between image versions. It uses Merkle Trees for secure and efficient version tracking, grayscale conversion to reduce data complexity, and Quadtrees to break down images into smaller chunks.

## Getting Started

### Installation

Please refer to the [Building Guide](BUILDING.md) for instructions on how to build and install Versionary.

### Initializing a Repository

Before you can start using Versionary, you need to initialize a repository:

```bash
versionary --cli init
```

Or, in GUI mode, select "File > Create Repository" and choose a directory.

## Command-Line Interface (CLI)

Versionary provides a command-line interface for managing image versions.

### Adding an Image

To add an image to the staging area:

```bash
versionary --cli add <image_path>
```

### Committing an Image

To commit the staged image:

```bash
versionary --cli commit "<message>" [branch] [encrypt] [sign]
```

Options:
- `branch`: Branch to commit to (default: current branch)
- `encrypt`: Whether to encrypt the image (true/false)
- `sign`: Whether to sign the version (true/false)

Example with encryption and signing:

```bash
versionary --cli commit "Added logo" main true true
```

### Listing Versions

To list all versions:

```bash
versionary --cli list
```

### Showing Version Information

To show information about a specific version:

```bash
versionary --cli show <version_id>
```

### Comparing Versions

To compare two versions and save the difference visualization:

```bash
versionary --cli compare <version_id1> <version_id2> <output_path>
```

### Rolling Back to a Previous Version

To roll back to a previous version:

```bash
versionary --cli rollback <version_id> [branch]
```

Options:
- `branch`: Branch to update (default: current branch)

### Visualizing Quadtree Structure

To visualize the Quadtree structure of a version:

```bash
versionary --cli visualize <version_id> <output_path>
```

## Graphical User Interface (GUI)

Versionary also provides a graphical user interface for managing image versions.

### Starting the GUI

To start the GUI:

```bash
versionary --gui
```

### Creating or Opening a Repository

1. Select "File > Create Repository" to create a new repository.
2. Select "File > Open Repository" to open an existing repository.

### Adding an Image

1. Click the "Add Image" button.
2. Select an image file from the file dialog.

### Committing an Image

1. Enter a commit message in the text field.
2. Optionally, check the "Encrypt" and/or "Sign" checkboxes.
3. Click the "Commit" button.

### Viewing Versions

The left panel displays a list of all versions. Click on a version to view its image.

### Comparing Versions

1. Select a version from the list.
2. Select another version from the dropdown menu.
3. Click the "Compare" button to see the differences.

### Rolling Back to a Previous Version

1. Select a version from the list.
2. Click the "Rollback" button.

### Visualizing Quadtree Structure

1. Select a version from the list.
2. Click the "Visualize Quadtree" button.

## Branch Management

### Listing Branches

To list all branches:

```bash
versionary --cli branch
```

### Creating a Branch

To create a new branch:

```bash
versionary --cli create-branch <branch_name> [start_point] [description]
```

Options:
- `start_point`: Version ID to start the branch from (default: current version)
- `description`: Description of the branch

### Switching Branches

To switch to a different branch:

```bash
versionary --cli switch-branch <branch_name>
```

### Merging Branches

To merge a branch into the current branch:

```bash
versionary --cli merge <branch_name> [message]
```

Options:
- `message`: Merge commit message

### Deleting a Branch

To delete a branch:

```bash
versionary --cli delete-branch <branch_name>
```

## Security Features

### Encrypting Images

Versionary supports encrypting images using AES-256-CBC encryption. To encrypt an image, use the `encrypt` option when committing:

```bash
versionary --cli commit "Added sensitive image" main true false
```

### Digital Signatures

Versionary supports signing versions using RSA digital signatures. To sign a version, use the `sign` option when committing:

```bash
versionary --cli commit "Important update" main false true
```

### Verifying Signatures

To verify a version's signature:

```bash
versionary --cli verify <version_id>
```

## Technical Details

### Merkle Trees

Versionary uses Merkle Trees to efficiently track changes between image versions. A Merkle Tree is a binary tree where each leaf node contains the hash of a data block, and each non-leaf node contains the hash of its child nodes.

### Quadtrees

Versionary uses Quadtrees to break down images into smaller chunks. A Quadtree recursively divides an image into four quadrants until a specified depth or homogeneity criterion is met.

### Grayscale Conversion

Versionary converts images to grayscale to reduce data complexity and improve processing efficiency.

### Version Control

Versionary stores each version of an image along with metadata such as the version ID, parent ID, commit message, timestamp, and root hash of the Merkle Tree.

### Parallel Processing

Versionary uses parallel processing to improve performance when working with large images. The number of threads used is automatically determined based on the system's capabilities.

### Caching

Versionary implements caching to avoid redundant computations and improve performance when working with the same images multiple times.

## Troubleshooting

### Common Issues

#### Image Not Found

If you get an error saying that an image file was not found, make sure the file path is correct and the file exists.

#### Repository Not Initialized

If you get an error saying that the repository is not initialized, make sure you have run the `init` command or created a repository through the GUI.

#### Failed to Commit

If you get an error saying that the commit failed, make sure you have added an image to the staging area first.

## Support

If you encounter any issues or have any questions, please open an issue on the [GitHub repository](https://github.com/yourusername/versionary/issues).
