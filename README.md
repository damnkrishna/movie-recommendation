
# Movie Recommendation System

A C++ application that provides personalized movie recommendations using collaborative filtering.

## Features
- **User Management**: Add, modify, and view user profiles
- **Movie Database**: Search, browse, and add movies with genres
- **Rating System**: Rate movies on a 1-5 scale
- **Recommendations**: Get personalized movie suggestions based on similar users
- **Graph Visualization**: View user similarity connections
- **Data Operations**: Import/export functionality including MovieLens dataset support

## Technical Details
- Uses Jaccard similarity to find users with similar tastes
- Implements graph-based user similarity model
- Persistent data storage in CSV format
- Command-line interface with intuitive menu system

## Getting Started
1. Compile with any C++ compiler supporting C++11 or later
2. Run the executable
3. Import movie data or add movies manually
4. Create user profiles and add ratings
5. Generate personalized recommendations

The system works best with a substantial number of users and ratings to create meaningful similarity connections between users.
