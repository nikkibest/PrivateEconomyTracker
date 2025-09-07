# Personal Economy Tracker

## Overview
The Personal Economy Tracker is a C++ application designed to help users manage their personal finances by tracking income and expenses. The application provides a user-friendly graphical interface for entering transactions and visualizing financial data through time-series graphs.

## Features
- Enter and categorize financial transactions as income or expenses.
- View a time-series graph of income and expenses over time.
- Save and load transaction data for persistent storage.

## Project Structure
```
personal-economy-tracker
├── src
│   ├── main.cpp          # Entry point of the application
│   ├── app.h             # Header file for the App class
│   ├── app.cpp           # Implementation of the App class
│   ├── gui
│   │   ├── gui.h         # Header file for the GUI class
│   │   └── gui.cpp       # Implementation of the GUI class
│   ├── model
│   │   ├── transaction.h  # Header file for the Transaction class
│   │   └── transaction.cpp# Implementation of the Transaction class
│   ├── data
│   │   ├── storage.h     # Header file for the Storage class
│   │   └── storage.cpp   # Implementation of the Storage class
│   └── utils
│       ├── plot.h        # Header file for the Plot class
│       └── plot.cpp      # Implementation of the Plot class
├── CMakeLists.txt        # CMake configuration file
└── README.md              # Project documentation
```

## Setup Instructions
1. Clone the repository:
   ```
   git clone <repository-url>
   cd personal-economy-tracker
   ```

2. Install the required dependencies:
   - Ensure you have CMake and a compatible C++ compiler installed.
   - Install GLFW, GLAD, and ImGui libraries.

3. Build the project:
   ```
   mkdir build
   cd build
   cmake ..
   make
   ```

4. Run the application:
   ```
   ./personal-economy-tracker
   ```

## Usage Guidelines
- Launch the application and use the GUI to enter your income and expenses.
- View the graphical representation of your financial data over time.
- Save your transactions to keep a record of your financial history.

## Contributing
Contributions are welcome! Please open an issue or submit a pull request for any enhancements or bug fixes.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.