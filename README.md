# Request Organizer

A Qt6-based request organizer application similar to Burp Suite's organizer.

## Features

- Organize requests in a hierarchical folder structure
- Create folders and organize requests by folders
- Set colors for request items (not folders)
- Add annotations to all items and folders
- Collapsible/expandable folders
- Table view with small fonts for compact display

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Requirements

- CMake 3.16 or higher
- Qt6 (Core and Widgets components)
- C++17 compatible compiler

## Usage

- **Add Folder**: Click "Add Folder" button or right-click and select "Add Folder"
- **Add Request**: Click "Add Request" button or right-click and select "Add Request"
- **Set Color**: Select a request item and click "Set Color" (folders cannot have colors)
- **Edit Annotation**: Select an item and click "Edit Annotation"
- **Delete**: Select an item and click "Delete"
- **Rename**: Double-click on the name column to edit
