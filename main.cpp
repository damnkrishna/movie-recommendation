
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <iomanip>
#include <queue>
#include <stack>
#include <unordered_set>
#include <ctime> 

using namespace std;


class Movie;
class User;
class Rating;
class Graph;
class RecommendationEngine;
class DataManager;
class UI;



long long getCurrentTimestamp()
{
    return chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
}

class Rating
{
public:
    int userId;
    int movieId;
    int ratingValue; // 1-5
    long long timestamp;

    Rating(int uId = 0, int mId = 0, int rating = 0, long long ts = 0)
        : userId(uId), movieId(mId), ratingValue(rating), timestamp(ts) {}
};

class Movie
{
public:
    int movieId;
    string title;
    vector<string> genres;
    int releaseYear;
    string description;

    Movie(int id = 0, string t = "", int year = 0, string desc = "")
        : movieId(id), title(t), releaseYear(year), description(desc) {}

    void addGenre(const string &genre)
    {
        if (find(genres.begin(), genres.end(), genre) == genres.end())
        {
            genres.push_back(genre);
        }
    }

    string genresToString() const
    {
        string result = "";
        for (size_t i = 0; i < genres.size(); ++i)
        {
            result += genres[i] + (i == genres.size() - 1 ? "" : "|");
        }
        return result;
    }

    void setGenresFromString(const string &genreStr)
    {
        genres.clear();
        stringstream ss(genreStr);
        string genre;
        while (getline(ss, genre, '|'))
        {
            if (!genre.empty())
            {
                addGenre(genre);
            }
        }
    }

    void display() const
    {
        cout << "ID: " << movieId << ", Title: " << title << " (" << releaseYear << ")" << endl;
        cout << "   Genres: " << genresToString() << endl;
        cout << "   Description: " << description << endl;
    }
};

class User
{
public:
    int userId;
    string name;
    unordered_map<int, Rating> ratings; 

    User(int id = 0, string n = "") : userId(id), name(n) {}

    bool addRating(int movieId, int ratingValue, const Movie &movie)
    {
        if (ratingValue < 1 || ratingValue > 5)
        {
            cerr << "Error: Rating must be between 1 and 5." << endl;
            return false;
        }
        if (ratings.count(movieId))
        {
            cout << "Updating existing rating for movie ID " << movieId << "." << endl;
        }
        ratings[movieId] = Rating(userId, movieId, ratingValue, getCurrentTimestamp());
        return true;
    }

    bool modifyRating(int movieId, int newRatingValue)
    {
        if (ratings.count(movieId))
        {
            if (newRatingValue < 1 || newRatingValue > 5)
            {
                cerr << "Error: New rating must be between 1 and 5." << endl;
                return false;
            }
            ratings[movieId].ratingValue = newRatingValue;
            ratings[movieId].timestamp = getCurrentTimestamp();
            cout << "Rating updated successfully." << endl;
            return true;
        }
        else
        {
            cerr << "Error: User has not rated this movie." << endl;
            return false;
        }
    }

    void displayRatings() const
    {
        if (ratings.empty())
        {
            cout << name << " has not rated any movies yet." << endl;
            return;
        }
        cout << name << "'s Ratings:" << endl;
        cout << left << setw(10) << "MovieID" << setw(50) << "Title (Placeholder)" << setw(8) << "Rating" << endl;
        cout << string(70, '-') << endl;
        for (const auto &pair : ratings)
        {
            cout << left << setw(10) << pair.first << setw(50) << "..." << setw(8) << pair.second.ratingValue << endl;
           
        }
    }

    void displayProfile() const
    {
        cout << "User ID: " << userId << ", Name: " << name << endl;
    }
};

class Graph
{
public:
    unordered_map<int, unordered_map<int, double>> adjList; 

    void addUserNode(int userId)
    {
        if (adjList.find(userId) == adjList.end())
        {
            adjList[userId] = unordered_map<int, double>();
        }
    }

    void addEdge(int userId1, int userId2, double similarity)
    {
        if (userId1 == userId2)
            return;
        adjList[userId1][userId2] = similarity;
        adjList[userId2][userId1] = similarity;
    }

    unordered_map<int, double> getNeighbors(int userId) const
    {
        if (adjList.count(userId))
        {
            return adjList.at(userId);
        }
        return {};
    }

    double getSimilarity(int userId1, int userId2) const
    {
        if (adjList.count(userId1) && adjList.at(userId1).count(userId2))
        {
            return adjList.at(userId1).at(userId2);
        }
        return 0.0;
    }

    vector<int> bfs(int startNode, int maxDistance) const
    {
        vector<int> reachableUsers;
        if (!adjList.count(startNode))
            return reachableUsers;

        queue<pair<int, int>> q; 
        unordered_set<int> visited;

        q.push({startNode, 0});
        visited.insert(startNode);

        while (!q.empty())
        {
            pair<int, int> current = q.front();
            q.pop();
            int currentUser = current.first;
            int currentDistance = current.second;

            if (currentUser != startNode)
            {
                reachableUsers.push_back(currentUser);
            }

            if (currentDistance >= maxDistance)
                continue;

            if (adjList.count(currentUser))
            {
                for (const auto &edge : adjList.at(currentUser))
                {
                    int neighbor = edge.first;
                    if (visited.find(neighbor) == visited.end())
                    {
                        visited.insert(neighbor);
                        q.push({neighbor, currentDistance + 1});
                    }
                }
            }
        }
        return reachableUsers;
    }

    vector<int> dfs(int startNode) const
    {
        vector<int> componentUsers;
        if (!adjList.count(startNode))
            return componentUsers;

        stack<int> s;
        unordered_set<int> visited;

        s.push(startNode);
        visited.insert(startNode);

        while (!s.empty())
        {
            int currentUser = s.top();
            s.pop();
            componentUsers.push_back(currentUser);

            if (adjList.count(currentUser))
            {
                for (const auto &edge : adjList.at(currentUser))
                {
                    int neighbor = edge.first;
                    if (visited.find(neighbor) == visited.end())
                    {
                        visited.insert(neighbor);
                        s.push(neighbor);
                    }
                }
            }
        }
        return componentUsers;
    }

    void displayGraph() const
    {
        cout << "\n--- User Similarity Graph ---" << endl;
        if (adjList.empty())
        {
            cout << "Graph is empty." << endl;
            return;
        }
        for (const auto &pair : adjList)
        {
            cout << "User " << pair.first << " connections:" << endl;
            if (pair.second.empty())
            {
                cout << "  (No similar users found based on threshold)" << endl;
            }
            else
            {
                for (const auto &edge : pair.second)
                {
                    cout << "  -> User " << edge.first << " (Similarity: " << fixed << setprecision(3) << edge.second << ")" << endl;
                }
            }
        }
        cout << "---------------------------\n"
             << endl;
    }
};

class DataManager
{
public:
    unordered_map<int, User> users;
    unordered_map<int, Movie> movies;
    int nextUserId = 1;
    int nextMovieId = 1;

    const string USERS_FILE = "users.csv";
    const string MOVIES_FILE = "movies.csv";
    const string RATINGS_FILE = "ratings.csv";

    // --- User Management ---
    bool addUser(const string &name)
    {
        for (const auto &pair : users)
        {
            if (pair.second.name == name)
            {
                cerr << "Error: User with name '" << name << "' already exists (ID: " << pair.first << ")." << endl;
                return false;
            }
        }
        User newUser(nextUserId, name);
        users[nextUserId] = newUser;
        cout << "User '" << name << "' added successfully with ID " << nextUserId << "." << endl;
        nextUserId++;
        return true;
    }

    User *getUser(int userId)
    {
        if (users.count(userId))
        {
            return &users[userId];
        }
        return nullptr;
    }

    const User *getUser(int userId) const
    {
        if (users.count(userId))
        {
            return &users.at(userId);
        }
        return nullptr;
    }

    bool modifyUser(int userId, const string &newName)
    {
        User *user = getUser(userId);
        if (!user)
        {
            cerr << "Error: User ID " << userId << " not found." << endl;
            return false;
        }
        for (const auto &pair : users)
        {
            if (pair.first != userId && pair.second.name == newName)
            {
                cerr << "Error: Another user with name '" << newName << "' already exists." << endl;
                return false;
            }
        }
        user->name = newName;
        cout << "User ID " << userId << " name updated to '" << newName << "'." << endl;
        return true;
    }

    void viewUser(int userId) const
    {
        const User *user = getUser(userId);
        if (user)
        {
            user->displayProfile();
            user->displayRatings(); 
        }
        else
        {
            cerr << "Error: User ID " << userId << " not found." << endl;
        }
    }

    void listUsers() const
    {
        cout << "\n--- User List ---" << endl;
        if (users.empty())
        {
            cout << "No users in the system." << endl;
            return;
        }
        cout << left << setw(10) << "User ID" << setw(30) << "Name" << endl;
        cout << string(40, '-') << endl;
      
        map<int, const User *> sortedUsers;
        for (const auto &pair : users)
        {
            sortedUsers[pair.first] = &pair.second;
        }
        for (const auto &pair : sortedUsers)
        {
            cout << left << setw(10) << pair.first << setw(30) << pair.second->name << endl;
        }
        cout << "-----------------\n"
             << endl;
    }

    
    bool addMovie(const string &title, int releaseYear, const string &description, const vector<string> &genres)
    {
        for (const auto &pair : movies)
        {
            if (pair.second.title == title && pair.second.releaseYear == releaseYear)
            {
                cerr << "Error: Movie '" << title << "' (" << releaseYear << ") already exists (ID: " << pair.first << ")." << endl;
                return false;
            }
        }
        Movie newMovie(nextMovieId, title, releaseYear, description);
        for (const string &genre : genres)
        {
            newMovie.addGenre(genre);
        }
        movies[nextMovieId] = newMovie;
        cout << "Movie '" << title << "' added successfully with ID " << nextMovieId << "." << endl;
        nextMovieId++;
        return true;
    }

    Movie *getMovie(int movieId)
    {
        if (movies.count(movieId))
        {
            return &movies[movieId];
        }
        return nullptr;
    }

    const Movie *getMovie(int movieId) const
    {
        if (movies.count(movieId))
        {
            return &movies.at(movieId);
        }
        return nullptr;
    }

    vector<const Movie *> searchMovies(const string &query) const
    {
        vector<const Movie *> results;
        string lowerQuery = query;
        transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

        for (const auto &pair : movies)
        {
            string lowerTitle = pair.second.title;
            transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
            if (lowerTitle.find(lowerQuery) != string::npos)
            {
                results.push_back(&pair.second);
            }
            else
            {
                for (const string &genre : pair.second.genres)
                {
                    string lowerGenre = genre;
                    transform(lowerGenre.begin(), lowerGenre.end(), lowerGenre.begin(), ::tolower);
                    if (lowerGenre.find(lowerQuery) != string::npos)
                    {
                        results.push_back(&pair.second);
                        break;
                    }
                }
            }
        }
        return results;
    }

    void listMovies(int page = 1, int pageSize = 10) const
    {
        cout << "\n--- Movie List ---" << endl;
        if (movies.empty())
        {
            cout << "No movies in the system." << endl;
            return;
        }

        vector<pair<int, const Movie *>> sortedMovies;
        for (const auto &pair : movies)
        {
            sortedMovies.push_back({pair.first, &pair.second});
        }
        sort(sortedMovies.begin(), sortedMovies.end()); 

        int totalMovies = sortedMovies.size();
        int totalPages = (totalMovies + pageSize - 1) / pageSize;
        if (page < 1 || page > totalPages)
        {
            cout << "Invalid page number. Displaying page 1." << endl;
            page = 1;
        }

        int startIdx = (page - 1) * pageSize;
        int endIdx = min(startIdx + pageSize, totalMovies);

        cout << "Showing page " << page << " of " << totalPages << " (" << totalMovies << " total movies)" << endl;
        cout << left << setw(10) << "ID" << setw(50) << "Title" << setw(8) << "Year" << "Genres" << endl;
        cout << string(100, '-') << endl;

        for (int i = startIdx; i < endIdx; ++i)
        {
            const Movie *movie = sortedMovies[i].second;
            cout << left << setw(10) << movie->movieId
                 << setw(50) << movie->title.substr(0, 47) + (movie->title.length() > 47 ? "..." : "")
                 << setw(8) << movie->releaseYear
                 << movie->genresToString().substr(0, 30) + (movie->genresToString().length() > 30 ? "..." : "") << endl;
        }
        cout << string(100, '-') << endl;
        if (totalPages > 1)
        {
            cout << "Use 'list movies [page_number]' to see other pages." << endl;
        }
        cout << "------------------\n"
             << endl;
    }

  
    bool addOrUpdateRating(int userId, int movieId, int ratingValue)
    {
        User *user = getUser(userId);
        const Movie *movie = getMovie(movieId);

        if (!user)
        {
            cerr << "Error: User ID " << userId << " not found." << endl;
            return false;
        }
        if (!movie)
        {
            cerr << "Error: Movie ID " << movieId << " not found." << endl;
            return false;
        }

        return user->addRating(movieId, ratingValue, *movie);  
    }

    
    double calculateJaccardSimilarity(int userId1, int userId2) const
    {
        const User *user1 = getUser(userId1);
        const User *user2 = getUser(userId2);

        if (!user1 || !user2)
            return 0.0;

        unordered_set<int> movies1;
        for (const auto &pair : user1->ratings)
        {
            movies1.insert(pair.first);
        }

        unordered_set<int> movies2;
        for (const auto &pair : user2->ratings)
        {
            movies2.insert(pair.first);
        }

        if (movies1.empty() || movies2.empty())
            return 0.0;

        unordered_set<int> intersection;
        unordered_set<int> unionSet = movies1; 

        for (int movieId : movies2)
        {
            if (movies1.count(movieId))
            {
                intersection.insert(movieId);
            }
            unionSet.insert(movieId); 
        }

        if (unionSet.empty())
            return 0.0; 

        return static_cast<double>(intersection.size()) / unionSet.size();
    }

    double calculateCommonRatingsSimilarity(int userId1, int userId2) const
    {
        const User *user1 = getUser(userId1);
        const User *user2 = getUser(userId2);

        if (!user1 || !user2)
            return 0.0;

        int commonCount = 0;
        for (const auto &pair1 : user1->ratings)
        {
            if (user2->ratings.count(pair1.first))
            {
                commonCount++;
            }
        }
       
        return static_cast<double>(commonCount);
    }

    void buildSimilarityGraph(Graph &graph, double jaccardThreshold = 0.1, double commonRatingThreshold = 2.0)
    {
        cout << "Building user similarity graph..." << endl;
        vector<int> userIds;
        for (const auto &pair : users)
        {
            userIds.push_back(pair.first);
            graph.addUserNode(pair.first); 
        }

        for (size_t i = 0; i < userIds.size(); ++i)
        {
            for (size_t j = i + 1; j < userIds.size(); ++j)
            {
                int u1 = userIds[i];
                int u2 = userIds[j];

                double jaccardSim = calculateJaccardSimilarity(u1, u2);
                double commonSim = calculateCommonRatingsSimilarity(u1, u2);

                
                if (jaccardSim >= jaccardThreshold && commonSim >= commonRatingThreshold)
                {
                    graph.addEdge(u1, u2, jaccardSim); 
                }
                
            }
        }
        cout << "Similarity graph built." << endl;
       
    }

    
    bool saveData() const
    {
        ofstream userFile(USERS_FILE);
        if (!userFile.is_open())
        {
            cerr << "Error: Could not open " << USERS_FILE << " for writing." << endl;
            return false;
        }
        userFile << "userId,name\n";
        for (const auto &pair : users)
        {
            userFile << pair.second.userId << "," << pair.second.name << "\n";
        }
        userFile.close();
        cout << "User data saved to " << USERS_FILE << endl;

        ofstream movieFile(MOVIES_FILE);
        if (!movieFile.is_open())
        {
            cerr << "Error: Could not open " << MOVIES_FILE << " for writing." << endl;
            return false;
        }
        movieFile << "movieId,title,releaseYear,genres,description\n";
        for (const auto &pair : movies)
        {
            movieFile << pair.second.movieId << ",\"" << pair.second.title << "\","
                      << pair.second.releaseYear << ",\"" << pair.second.genresToString() << "\",\""
                      << pair.second.description << "\"\n";
        }
        movieFile.close();
        cout << "Movie data saved to " << MOVIES_FILE << endl;

        ofstream ratingFile(RATINGS_FILE);
        if (!ratingFile.is_open())
        {
            cerr << "Error: Could not open " << RATINGS_FILE << " for writing." << endl;
            return false;
        }
        ratingFile << "userId,movieId,rating,timestamp\n";
        for (const auto &userPair : users)
        {
            for (const auto &ratingPair : userPair.second.ratings)
            {
                const Rating &r = ratingPair.second;
                ratingFile << r.userId << "," << r.movieId << "," << r.ratingValue << "," << r.timestamp << "\n";
            }
        }
        ratingFile.close();
        cout << "Rating data saved to " << RATINGS_FILE << endl;

        return true;
    }

    
    vector<string> parseCsvLine(const string &line) const
    {
        vector<string> fields;
        stringstream ss(line);
        string field;
        bool in_quotes = false;
        char c;

        while (ss.get(c))
        {
            if (c == '"')
            {
                in_quotes = !in_quotes;
            }
            else if (c == ',' && !in_quotes)
            {
                fields.push_back(field);
                field.clear();
            }
            else
            {
                field += c;
            }
        }
        fields.push_back(field); 
        return fields;
    }

    bool loadData()
    {
        ifstream userFile(USERS_FILE);
        int maxUserId = 0;
        if (userFile.is_open())
        {
            string line;
            getline(userFile, line); 
            while (getline(userFile, line))
            {
                stringstream ss(line);
                string segment;
                vector<string> parts;
                while (getline(ss, segment, ','))
                {
                    parts.push_back(segment);
                }
                if (parts.size() >= 2)
                {
                    try
                    {
                        int userId = stoi(parts[0]);
                        string name = parts[1];
                        users[userId] = User(userId, name);
                        maxUserId = max(maxUserId, userId);
                    }
                    catch (const std::exception &e)
                    {
                        cerr << "Warning: Skipping invalid user line: " << line << " (" << e.what() << ")" << endl;
                    }
                }
            }
            userFile.close();
            nextUserId = maxUserId + 1;
            cout << "User data loaded from " << USERS_FILE << ". Next User ID: " << nextUserId << endl;
        }
        else
        {
            cout << "User file (" << USERS_FILE << ") not found. Starting fresh." << endl;
        }

        ifstream movieFile(MOVIES_FILE);
        int maxMovieId = 0;
        if (movieFile.is_open())
        {
            string line;
            getline(movieFile, line);
            while (getline(movieFile, line))
            {
                vector<string> parts = parseCsvLine(line);

                if (parts.size() >= 5)
                {
                    try
                    {
                        int movieId = stoi(parts[0]);
                        string title = parts[1];
                        int year = stoi(parts[2]);
                        string genresStr = parts[3];
                        string description = parts[4];

                        
                        if (!title.empty() && title.front() == '"')
                            title = title.substr(1);
                        if (!title.empty() && title.back() == '"')
                            title.pop_back();
                        if (!genresStr.empty() && genresStr.front() == '"')
                            genresStr = genresStr.substr(1);
                        if (!genresStr.empty() && genresStr.back() == '"')
                            genresStr.pop_back();
                        if (!description.empty() && description.front() == '"')
                            description = description.substr(1);
                        if (!description.empty() && description.back() == '"')
                            description.pop_back();

                        movies[movieId] = Movie(movieId, title, year, description);
                        movies[movieId].setGenresFromString(genresStr);
                        maxMovieId = max(maxMovieId, movieId);
                    }
                    catch (const std::exception &e)
                    {
                        cerr << "Warning: Skipping invalid movie line: " << line << " (" << e.what() << ")" << endl;
                    }
                }
                else
                {
                    cerr << "Warning: Skipping malformed movie line: " << line << endl;
                }
            }
            movieFile.close();
            nextMovieId = maxMovieId + 1;
            cout << "Movie data loaded from " << MOVIES_FILE << ". Next Movie ID: " << nextMovieId << endl;
        }
        else
        {
            cout << "Movie file (" << MOVIES_FILE << ") not found. Starting fresh." << endl;
        }

        ifstream ratingFile(RATINGS_FILE);
        if (ratingFile.is_open())
        {
            string line;
            getline(ratingFile, line); 
            int count = 0;
            while (getline(ratingFile, line))
            {
                stringstream ss(line);
                string segment;
                vector<string> parts;
                while (getline(ss, segment, ','))
                {
                    parts.push_back(segment);
                }
                if (parts.size() >= 4)
                {
                    try
                    {
                        int userId = stoi(parts[0]);
                        int movieId = stoi(parts[1]);
                        int ratingVal = stoi(parts[2]);
                        long long timestamp = stoll(parts[3]);

                        if (users.count(userId) && movies.count(movieId))
                        {
                            users[userId].ratings[movieId] = Rating(userId, movieId, ratingVal, timestamp);
                            count++;
                        }
                        else
                        {
                            cerr << "Warning: Skipping rating for non-existent user/movie: " << line << endl;
                        }
                    }
                    catch (const std::exception &e)
                    {
                        cerr << "Warning: Skipping invalid rating line: " << line << " (" << e.what() << ")" << endl;
                    }
                }
            }
            ratingFile.close();
            cout << count << " ratings loaded from " << RATINGS_FILE << endl;
        }
        else
        {
            cout << "Rating file (" << RATINGS_FILE << ") not found. Starting fresh." << endl;
        }
        return true;
    }

    bool importMovieLensSmall(const string &moviesPath, const string &ratingsPath)
    {
        cout << "Importing MovieLens Small dataset..." << endl;
        ifstream movieFile(moviesPath);
        if (!movieFile.is_open())
        {
            cerr << "Error: Cannot open MovieLens movies file: " << moviesPath << endl;
            return false;
        }

        string line;
        getline(movieFile, line);
        int moviesImported = 0;
        int moviesSkipped = 0;
        while (getline(movieFile, line))
        {
            vector<string> parts = parseCsvLine(line);
            if (parts.size() >= 3)
            {
                try
                {
                    int mlMovieId = stoi(parts[0]);
                    string titleYear = parts[1];
                    string genresStr = parts[2];

                    string title = titleYear;
                    int year = 0;
                    size_t yearPos = titleYear.find_last_of('(');
                    if (yearPos != string::npos && titleYear.back() == ')')
                    {
                        try
                        {
                            year = stoi(titleYear.substr(yearPos + 1, 4));
                            title = titleYear.substr(0, yearPos);
                           
                            size_t lastChar = title.find_last_not_of(' ');
                            if (string::npos != lastChar)
                                title = title.substr(0, lastChar + 1);
                        }
                        catch (...)
                        {
                            year = 0;
                        } 
                    }

                    

                    Movie newMovie(nextMovieId, title, year, ""); 
                    newMovie.setGenresFromString(genresStr);
                    movies[nextMovieId] = newMovie;
                    nextMovieId++;
                    moviesImported++;
                }
                catch (const std::exception &e)
                {
                    cerr << "Warning: Skipping invalid MovieLens movie line: " << line << " (" << e.what() << ")" << endl;
                    moviesSkipped++;
                }
            }
            else
            {
                cerr << "Warning: Skipping malformed MovieLens movie line: " << line << endl;
                moviesSkipped++;
            }
        }
        movieFile.close();
        cout << "Imported " << moviesImported << " movies, skipped " << moviesSkipped << "." << endl;

       
        cout << "Warning: MovieLens rating import requires ID mapping, which is not fully implemented here." << endl;
        cout << "Only movies were imported. Please add ratings manually or implement ID mapping." << endl;

        return true; 
    }

    bool exportRecommendations(int userId, const vector<pair<double, const Movie *>> &recommendations, const string &filename) const
    {
        ofstream outFile(filename);
        if (!outFile.is_open())
        {
            cerr << "Error: Could not open file " << filename << " for exporting recommendations." << endl;
            return false;
        }

        const User *user = getUser(userId);
        outFile << "Top Recommendations for User: " << (user ? user->name : "ID " + to_string(userId)) << endl;
        outFile << "--------------------------------------------------" << endl;
        outFile << left << setw(12) << "Pred Rating" << setw(10) << "Movie ID" << setw(50) << "Title" << setw(8) << "Year" << "Genres" << endl;
        outFile << string(120, '-') << endl;

        for (const auto &rec : recommendations)
        {
            double predictedRating = rec.first;
            const Movie *movie = rec.second;
            if (movie)
            {
                outFile << left << fixed << setprecision(3) << setw(12) << predictedRating
                        << setw(10) << movie->movieId
                        << setw(50) << movie->title.substr(0, 47) + (movie->title.length() > 47 ? "..." : "")
                        << setw(8) << movie->releaseYear
                        << movie->genresToString() << endl;
            }
        }
        outFile.close();
        cout << "Recommendations exported to " << filename << endl;
        return true;
    }
};

struct RecommendationResult
{
    const Movie *movie;
    double predictedRating;
    vector<pair<int, double>> contributingUsers; 
};

class RecommendationEngine
{
    const DataManager &dataManager;
    const Graph &graph;

public:
    RecommendationEngine(const DataManager &dm, const Graph &g) : dataManager(dm), graph(g) {}

    vector<RecommendationResult> generateRecommendations(int userId, int N = 10, int neighborLimit = 50) const
    {
        vector<RecommendationResult> recommendations;
        const User *targetUser = dataManager.getUser(userId);
        if (!targetUser)
        {
            cerr << "Error: Target user ID " << userId << " not found." << endl;
            return recommendations;
        }

        unordered_map<int, double> similarUsers = graph.getNeighbors(userId);
        if (similarUsers.empty())
        {
            cout << "No similar users found for User ID " << userId << ". Cannot generate collaborative recommendations." << endl;
           
            return recommendations;
        }

        
        vector<pair<double, int>> sortedNeighbors; 
        for (const auto &pair : similarUsers)
        {
            sortedNeighbors.push_back({pair.second, pair.first});
        }
        sort(sortedNeighbors.rbegin(), sortedNeighbors.rend());
        if (sortedNeighbors.size() > neighborLimit)
        {
            sortedNeighbors.resize(neighborLimit);
        }

        map<int, pair<double, double>> movieScores;            
        map<int, vector<pair<int, double>>> movieContributors; 

        for (const auto &neighborPair : sortedNeighbors)
        {
            double similarity = neighborPair.first;
            int neighborId = neighborPair.second;
            const User *neighbor = dataManager.getUser(neighborId);

            if (!neighbor || similarity <= 0)
                continue; 

            for (const auto &ratingPair : neighbor->ratings)
            {
                int movieId = ratingPair.first;
                int neighborRating = ratingPair.second.ratingValue;

                
                if (targetUser->ratings.count(movieId))
                {
                    continue; 
                }

                
                movieScores[movieId].first += similarity * neighborRating;
                movieScores[movieId].second += similarity;
                movieContributors[movieId].push_back({neighborId, similarity});
            }
        }

        
        vector<pair<double, int>> potentialRecs; 
        for (const auto &scorePair : movieScores)
        {
            int movieId = scorePair.first;
            double weightedSum = scorePair.second.first;
            double similaritySum = scorePair.second.second;

            if (similaritySum > 0)
            { 
                double predictedRating = weightedSum / similaritySum;
                potentialRecs.push_back({predictedRating, movieId});
            }
        }
        sort(potentialRecs.rbegin(), potentialRecs.rend());
        int count = 0;
        for (const auto &recPair : potentialRecs)
        {
            if (count >= N)
                break;
            double predictedRating = recPair.first;
            int movieId = recPair.second;
            const Movie *movie = dataManager.getMovie(movieId);

            if (movie)
            {
                RecommendationResult result;
                result.movie = movie;
                result.predictedRating = predictedRating;
                if (movieContributors.count(movieId))
                {
                    result.contributingUsers = movieContributors[movieId];
                    
                    sort(result.contributingUsers.rbegin(), result.contributingUsers.rend(),
                         [](const pair<int, double> &a, const pair<int, double> &b)
                         {
                             return a.second < b.second; 
                         });
                }
                recommendations.push_back(result);
                count++;
            }
        }

        return recommendations;
    }

    void displayRecommendations(int userId, const vector<RecommendationResult> &recommendations) const
    {
        cout << "\n--- Top " << recommendations.size() << " Recommendations for User ID " << userId << " ---" << endl;
        const User *user = dataManager.getUser(userId);
        if (user)
            cout << "--- User: " << user->name << " ---" << endl;

        if (recommendations.empty())
        {
            cout << "No recommendations could be generated." << endl;
            return;
        }

        cout << left << setw(12) << "Pred Rating" << setw(10) << "Movie ID" << setw(50) << "Title" << "Explanation (Top Contributors)" << endl;
        cout << string(150, '-') << endl;

        for (const auto &rec : recommendations)
        {
            cout << left << fixed << setprecision(3) << setw(12) << rec.predictedRating
                 << setw(10) << rec.movie->movieId
                 << setw(50) << rec.movie->title.substr(0, 47) + (rec.movie->title.length() > 47 ? "..." : "");

            
            string explanation = " Rated highly by: ";
            int contributorsShown = 0;
            for (const auto &contributor : rec.contributingUsers)
            {
                if (contributorsShown >= 3)
                {
                    explanation += "...";
                    break;
                }
                const User *contribUser = dataManager.getUser(contributor.first);
                explanation += (contribUser ? contribUser->name : "ID " + to_string(contributor.first));
                explanation += " (Sim: " + to_string(contributor.second).substr(0, 4) + "), ";
                contributorsShown++;
            }
            if (!rec.contributingUsers.empty())
                explanation.pop_back();
            explanation.pop_back();
            cout << explanation << endl;
        }
        cout << string(150, '-') << endl;
    }
};

class UI
{
    DataManager &dataManager;
    Graph &graph;
    RecommendationEngine &engine;

    void displayMainMenu()
    {
        cout << "\n===== Movie Recommendation Engine =====" << endl;
        cout << "1. User Management" << endl;
        cout << "2. Movie Management" << endl;
        cout << "3. Rate a Movie" << endl;
        cout << "4. Get Recommendations" << endl;
        cout << "5. Data Operations" << endl;
        cout << "6. View Similarity Graph" << endl;
        cout << "0. Save and Exit" << endl;
        cout << "=====================================" << endl;
        cout << "Enter your choice: ";
    }

    void displaySubMenu(const string &title, const vector<string> &options)
    {
        cout << "\n--- " << title << " ---" << endl;
        for (size_t i = 0; i < options.size(); ++i)
        {
            cout << i + 1 << ". " << options[i] << endl;
        }
        cout << "0. Back to Main Menu" << endl;
        cout << "--------------------" << endl;
        cout << "Enter your choice: ";
    }

    // --- Input Helpers ---
    int getIntInput(const string &prompt)
    {
        int value;
        cout << prompt;
        while (!(cin >> value))
        {
            cout << "Invalid input. Please enter an integer: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Consume newline
        return value;
    }

    string getStringInput(const string &prompt)
    {
        string value;
        cout << prompt;
        getline(cin, value);
        return value;
    }

    // --- Menu Handlers ---
    void handleUserManagement()
    {
        vector<string> options = {"Add User", "Modify User", "View User Profile", "List All Users"};
        int choice;
        do
        {
            displaySubMenu("User Management", options);
            choice = getIntInput("");
            switch (choice)
            {
            case 1:
            {
                string name = getStringInput("Enter new user name: ");
                if (!name.empty())
                    dataManager.addUser(name);
                dataManager.buildSimilarityGraph(graph); // Rebuild graph if users change
                break;
            }
            case 2:
            {
                int userId = getIntInput("Enter User ID to modify: ");
                string newName = getStringInput("Enter new name: ");
                if (!newName.empty())
                    dataManager.modifyUser(userId, newName);
                break;
            }
            case 3:
            {
                int userId = getIntInput("Enter User ID to view: ");
                dataManager.viewUser(userId);
                break;
            }
            case 4:
                dataManager.listUsers();
                break;
            case 0:
                cout << "Returning to main menu..." << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
            }
        } while (choice != 0);
    }

    void handleMovieManagement()
    {
        vector<string> options = {"Add Movie", "Search Movies", "View Movie Details", "List All Movies"};
        int choice;
        do
        {
            displaySubMenu("Movie Management", options);
            choice = getIntInput("");
            switch (choice)
            {
            case 1:
            {
                string title = getStringInput("Enter movie title: ");
                int year = getIntInput("Enter release year: ");
                string desc = getStringInput("Enter description: ");
                string genresStr = getStringInput("Enter genres (comma-separated): ");
                vector<string> genres;
                stringstream ss(genresStr);
                string genre;
                while (getline(ss, genre, ','))
                {
                    // Trim whitespace
                    genre.erase(0, genre.find_first_not_of(" \t\n\r\f\v"));
                    genre.erase(genre.find_last_not_of(" \t\n\r\f\v") + 1);
                    if (!genre.empty())
                        genres.push_back(genre);
                }
                if (!title.empty())
                    dataManager.addMovie(title, year, desc, genres);
                break;
            }
            case 2:
            {
                string query = getStringInput("Enter search query (title or genre): ");
                vector<const Movie *> results = dataManager.searchMovies(query);
                if (results.empty())
                {
                    cout << "No movies found matching '" << query << "'." << endl;
                }
                else
                {
                    cout << "\n--- Search Results ---" << endl;
                    for (const Movie *movie : results)
                    {
                        cout << "ID: " << movie->movieId << " - " << movie->title << " (" << movie->releaseYear << ") [" << movie->genresToString() << "]" << endl;
                    }
                    cout << "--------------------" << endl;
                }
                break;
            }
            case 3:
            {
                int movieId = getIntInput("Enter Movie ID to view details: ");
                const Movie *movie = dataManager.getMovie(movieId);
                if (movie)
                {
                    movie->display();
                }
                else
                {
                    cerr << "Error: Movie ID " << movieId << " not found." << endl;
                }
                break;
            }
            case 4:
            {
                int page = getIntInput("Enter page number to display (or 1 for first page): ");
                dataManager.listMovies(page);
                break;
            }
            case 0:
                cout << "Returning to main menu..." << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
            }
        } while (choice != 0);
    }

    void handleRateMovie()
    {
        cout << "\n--- Rate a Movie ---" << endl;
        int userId = getIntInput("Enter your User ID: ");
        if (!dataManager.getUser(userId))
        {
            cerr << "Error: User ID " << userId << " not found." << endl;
            return;
        }
        cout << "\nAvailable Movies:" << endl;
        cout << left << setw(10) << "ID" << setw(50) << "Title" << setw(8) << "Year" << "Genres" << endl;
        cout << string(100, '-') << endl;
        
        
        vector<pair<int, const Movie*>> sortedMovies;
        for (const auto &pair : dataManager.movies)
        {
            sortedMovies.push_back({pair.first, &pair.second});
        }
        sort(sortedMovies.begin(), sortedMovies.end()); 
        
        for (const auto &pair : sortedMovies)
        {
            const Movie *movie = pair.second;
            cout << left << setw(10) << movie->movieId
                 << setw(50) << movie->title.substr(0, 47) + (movie->title.length() > 47 ? "..." : "")
                 << setw(8) << movie->releaseYear
                 << movie->genresToString().substr(0, 30) + (movie->genresToString().length() > 30 ? "..." : "") << endl;
        }
        cout << endl;
        
        int movieId = getIntInput("Enter Movie ID to rate: ");
        if (!dataManager.getMovie(movieId))
        {
            cerr << "Error: Movie ID " << movieId << " not found." << endl;
            return;
        }
        int rating = getIntInput("Enter your rating (1-5): ");
    
        if (dataManager.addOrUpdateRating(userId, movieId, rating))
        {
            cout << "Rating added/updated successfully." << endl;
            
            dataManager.buildSimilarityGraph(graph, 0.1, 1.0); // Use slightly lower common rating threshold maybe?
        }
        else
        {
            cout << "Failed to add/update rating." << endl;
        }
    }

    void handleGetRecommendations()
    {
        cout << "\n--- Get Movie Recommendations ---" << endl;
        int userId = getIntInput("Enter your User ID: ");
        if (!dataManager.getUser(userId))
        {
            cerr << "Error: User ID " << userId << " not found." << endl;
            return;
        }

        int N = getIntInput("Enter number of recommendations desired (e.g., 10): ");
        if (N <= 0)
            N = 10;

        vector<RecommendationResult> recs = engine.generateRecommendations(userId, N);
        engine.displayRecommendations(userId, recs);

        if (!recs.empty())
        {
            string exportChoice = getStringInput("Export these recommendations to a file? (y/n): ");
            if (exportChoice == "y" || exportChoice == "Y")
            {
                string filename = "recommendations_user_" + to_string(userId) + ".txt";
                vector<pair<double, const Movie *>> exportRecs;
                for (const auto &rec : recs)
                {
                    exportRecs.push_back({rec.predictedRating, rec.movie});
                }

                dataManager.exportRecommendations(userId, exportRecs, filename);
            }
        }
    }

    void handleDataOperations()
    {
        vector<string> options = {"Save All Data", "Load All Data", "Import MovieLens Small Dataset"};
        int choice;
        do
        {
            displaySubMenu("Data Operations", options);
            choice = getIntInput("");
            switch (choice)
            {
            case 1:
                dataManager.saveData();
                break;
            case 2:
                if (dataManager.loadData())
                {
                    // Rebuild graph after loading new data
                    dataManager.buildSimilarityGraph(graph);
                }
                break;
            case 3:
            {
                string moviesPath = getStringInput("Enter path to MovieLens movies.csv: ");
                string ratingsPath = getStringInput("Enter path to MovieLens ratings.csv: ");
                if (dataManager.importMovieLensSmall(moviesPath, ratingsPath))
                {
                    dataManager.buildSimilarityGraph(graph); // Rebuild graph after import
                }
                break;
            }
            case 0:
                cout << "Returning to main menu..." << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
            }
        } while (choice != 0);
    }

public:
    UI(DataManager &dm, Graph &g, RecommendationEngine &re) : dataManager(dm), graph(g), engine(re) {}

    void run()
    {
        // Initial load and graph build
        dataManager.loadData();
        dataManager.buildSimilarityGraph(graph);

        int choice;
        do
        {
            displayMainMenu();
            choice = getIntInput("");

            switch (choice)
            {
            case 1:
                handleUserManagement();
                break;
            case 2:
                handleMovieManagement();
                break;
            case 3:
                handleRateMovie();
                break;
            case 4:
                handleGetRecommendations();
                break;
            case 5:
                handleDataOperations();
                break;
            case 6:
                graph.displayGraph();
                break;
            case 0:
                cout << "Saving data before exiting..." << endl;
                dataManager.saveData();
                cout << "Exiting program. Goodbye!" << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
            }
        } while (choice != 0);
    }
};

// --- Main Function ---
int main()
{
    DataManager dataManager;
    Graph graph;
    RecommendationEngine engine(dataManager, graph);
    UI ui(dataManager, graph, engine);

    ui.run();

    return 0;
}
