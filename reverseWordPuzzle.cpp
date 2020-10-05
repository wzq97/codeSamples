#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <assert.h>
#include <algorithm>
typedef std::vector<std::vector<char> > Board;	//representation of the 2d vector

// Word class contains a value that is a positive word,
// and a bool used determines whether this word is used in the board
class Word{
public:
	Word(std::string word, bool u):value(word), used(u){}
	std::string value;
	bool used;
};

// Reverse_word function returns a word after reverse its letters
const std::string reverse_word(const std::string& word){
	std::string answer;
	for (int i = word.size()-1; i >= 0; i--){
		answer.push_back(word[i]);
	}
	return answer;
}

// is_complete is called in the base case to check if every spot in
// board is filled with letter. Return false if any spot is empty.
bool is_complete(Board& board){
	for(int i = 0; i < board.size();i++){
		for(int j = 0; j < board[i].size(); j++){
			if(board[i][j] == ' ') return false;
		}
	}
	return true;
}

// Add_word function takes the arguments
//			board: the current board 
//			r, c: the row and col number
//			direction: the direction word needed to be put
//			word: the word that is going to be added
//			positive: the + words vector
// This function search and check each spot that the word is going
// to be added. If any spot is not valid(not a empty space and do not 
// match with the word letter) then return false. Otherwise add it 
// to the board. 
bool add_word(Board& board, 
				 int r, int c,			//location to start
				 const std::string direction, 
				 const std::string word,
				 std::vector<Word>& positive){
	int tmp;
	std::string rword = reverse_word(word);
	for(int k = 0; k<positive.size();k++){
		if(positive[k].value == word || positive[k].value == rword ) {
			if(positive[k].used) {
				return false;}//return false the + word is used
			tmp = k;
			positive[k].used = true;	// find the word in vector and mark it as used
			break;
		}
	}
	// Check and put in word in right direction
	if(direction == "r"){		
		for(int i = 0; i < word.size(); i++){
			if(board[r][c+i] != ' ' && board[r][c+i] != word[i] ) return false;
			if(board[r][c+i] == ' ') board[r][c+i] = word[i];//add letter to the board
			else if(board[r][c+i] == word[i]) continue;	//skip the letter if they are same
		}			
		return true;
	}
	// Check and put in word in left direction
	else if(direction == "l"){
		if(add_word(board,r,c-word.size()+1,"r", rword,positive)) return true;
	}
	// Check and put in word in down direction
	else if(direction == "d"){
		for(int j = 0; j < word.size(); j++){
			if( board[r+j][c] != ' ' && board[r+j][c] != word[j] ) return false;
			if(board[r+j][c] == ' ') board[r+j][c] = word[j];
			else if(board[r+j][c] == word[j]) continue;		
		}
		return true;
	}
	// Check and put in word in up direction
	else if(direction == "u"){
		if(add_word(board,r-word.size()+1,c,"d", rword,positive)) return true;
	}
	// Check and put in word in right down direction
	else if(direction == "rd"){
		for(int i = 0,j = 0; i < word.size(),j < word.size(); i++,j++){
			if( board[r+j][c+i] != ' ' && board[r+j][c+i] != word[j] ) return false;
			if(board[r+j][c+i] == ' ') board[r+j][c+i] = word[j];
			else if(board[r+j][c+i] == word[j]) continue;		
		}
		return true;
	}
	// Check and put in word in left down direction
	else if(direction == "ld"){
		for(int i = 0,j = 0; i < word.size(),j < word.size(); i++,j++){
			if( board[r+j][c-i] != ' ' && board[r+j][c-i] != word[j] ) return false;
			if(board[r+j][c-i] == ' ') board[r+j][c-i] = word[j];
			else if(board[r+j][c-i] == word[j]) continue;		
		}
		return true;
	}
	// Check and put in word in right up direction
	else if(direction == "ru"){
		if(add_word(board,r-word.size()+1,c+word.size()-1,"ld", rword,positive)) return true;
	}
	// Check and put in word in left up direction
	else if(direction == "lu"){
		if(add_word(board,r-word.size()+1,c-word.size()+1,"rd", rword,positive)) return true;
	}
	return false;
}

// Search_word function reads the current board, the word and a location
// search in the eight direction, if any direction matches with the second
// letter, continue in that direction. Return true if the word is found.
bool search_word(Board& board,int r, int c,std::string word){
	std::string path;

	if(r >= board.size() || c >= board[0].size()) return false;
	for (int i = r-1; i <= r+1; i++) {		// searching in 8 direction
	    for (int j = c-1; j <= c+1; j++) {
	      	if (i < 0 || i >= int(board.size())) continue;//stay in the bound
	      	if (j < 0 || j >= int(board[i].size())) continue;
	      	if (i == r && j == c) continue;		//skip the origin
	      	int row = r;
			int col = c;
			path = "";
	      	if (word[1] == board[i][j]) {// if matches with second letter of word
	      		int x = i-row;
	      		int y = j-col;
	      		for(int k = 0; k < word.size();k++){
	      			if (row < 0 || row >= int(board.size())) break;
	      			if (col < 0 || col >= int(board[i].size())) break;
	      			path += board[row][col];// add the letter to the temp string
	      			row += x;
	      			col += y;// continue in that direction
	      		}
	      		if(path == word) return true;//if they matches: the word is found
		    }
	    }
	}
	return false;
}

bool check_positive(Board& board,std::vector<Word>& positive){
	std::vector<Word> positive_;
	int count =0;
	bool flag = false;
	for(int p = 0; p < positive.size();p++){   //for every +word
		for(int i = 0; i<board.size();i++){
			for(int j = 0; j<board[i].size();j++){	
				if((positive[p].value)[0] == board[i][j]){
					if(search_word(board,i,j,positive[p].value)){
						count++;						
						flag = true;
					}
				}			
			}
		}
		if(!flag) return false; // did not find this + word
		flag = false;
	}
	return true;
}

// return true if contains negative, else return false;
bool check_negative(Board& board,std::vector<std::string>& negative){
	for(int n = 0; n < negative.size();n++){
		for(int i = 0; i<board.size();i++){
			for(int j = 0; j<board[i].size();j++){
				if(board[i][j] == ' ') continue;
				if(negative[n][0] == board[i][j]){
					if(negative[n].size() == 1) return true;
					if(search_word(board,i,j,negative[n]))	return true;
				}
			}
		}
	}
	return false;
}

// Compare two boards, if any spot is different return false. 
// Otherwise return true.
bool operator==(const Board& b1,const Board& b2){

	for(int i = 0; i < b1.size(); i++ ){
		for(int j = 0; j< b1[i].size(); j++){
			if(b1[i][j] != b2[i][j]) return false;
		}
	}
	return true;
}

// Check if one board is unique in the solutions. Return false if
// it has repeated. Otherwise return true.
bool is_unique(std::vector<Board>& allboards,Board& board){
	for (int b = 0; b< allboards.size(); b++){
		if(allboards[b] == board) return false;
	}
	return true;
}

// Create_board function takes arguments 
//		- allboards:contains all solutions
//		- board:current board
//		- positive:contains all positive words
//		- negative:all negative word trying wo avoid
// it recursively call the function and find all solutions that includes
// all the positive words while avoiding all negative words.			
bool create_board(std::vector<Board>& allboards,
				  Board& board,
				  std::vector<Word>& positive,
				  std::vector<std::string>& negative){
	std::string alphabet = "abcdefghijklmnopqrstuvwxyz";
	Board tmp;

	// BASE CASE: - no negative word
	// 			  - contains all positive word
	//			  - no empty spot
	//			  - is unique
	if(check_negative(board,negative)) return false;
	if(check_positive(board,positive) ){
		//print(board);
		if(is_complete(board) && is_unique(allboards,board)){
			allboards.push_back(board);	// add this board if it passes the base case
			return true;
		}
		// if all positive words are included but there is empty spot, add alphabet
		for(int i = 0; i < board.size(); i++){
			for(int j = 0; j < board[i].size(); j++){
				if(board[i][j] == ' '){
					for(int k = 0; k < alphabet.size(); k++){
						board[i][j] = alphabet[k];
						create_board(allboards,board,positive,negative);
						board[i][j] = ' ';
					}
				}
			}
		}
		return false;
	}

	// Other cases: search each spot and try to add word in
	int r, c;
	for(int i = 0; i < board.size(); i++){
		for(int j = 0; j < board[i].size(); j++){
			if (i < 0 || i >= int(board.size())) break;		// break if it is out of bound
	      	if (j < 0 || j >= int(board[i].size())) break;
			for(int k =0; k < positive.size(); k++){
				if (positive[k].used == true) continue;	// if this word is already used, skip it.
				// if this spot is emtpy or it the first letter of the word
				// then continue to add word and recurse
				if(board[i][j] == ' ' || board[i][j] == (positive[k].value)[0] ){	
					r=i;	
					c=j;
					std::string word = positive[k].value;
					std::string rword = reverse_word(word);
					// Put the word in "right" direction and do recursion
	      			if( word.size() <= board[i].size()-c){
	      				tmp = board;
	      				if(add_word(tmp,r,c,"r",word,positive))		//adding the word
	      					create_board(allboards,tmp,positive,negative);	    // recursion	
						positive[k].used = false;				//unmark the word
						if (rword != word){		
						tmp = board;
	      				if(add_word(tmp,r,c,"r",rword,positive))
	      					create_board(allboards,tmp,positive,negative);
						positive[k].used = false;}	
					}
					// Put the word in "left" direction and do recursion
					if( word.size() <= c+1 ){ 
						tmp = board;
	      				if(add_word(tmp,r,c,"l",word,positive)){
							create_board(allboards,tmp,positive,negative);
						}
						positive[k].used = false;	
						if (rword != word){
						tmp = board;
	      				if(add_word(tmp,r,c,"l",rword,positive)){
							create_board(allboards,tmp,positive,negative);
						}
						positive[k].used = false;}
					}		
					// Put the word in "right down" direction and do recursion
					if( word.size() <= board[i].size()-c && word.size() <= board.size()-r ){
						tmp = board;
	      				if(add_word(tmp,r,c,"rd",word,positive)){
							create_board(allboards,tmp,positive,negative);
						}
						positive[k].used = false;
						if (rword != word){	
						tmp = board;
	      				if(add_word(tmp,r,c,"rd",rword,positive)){
							create_board(allboards,tmp,positive,negative);
						}
						positive[k].used = false;}	
					}
					// Put the word in "left down" direction and do recursion
					if( word.size() <= c+1 && word.size() <= board.size()-r ){
						tmp = board;
	      				if(add_word(tmp,r,c,"ld",word,positive)){
							create_board(allboards,tmp,positive,negative);
						}
						positive[k].used = false;
						if (rword != word){	
						tmp = board;
	      				if(add_word(tmp,r,c,"ld",rword,positive)){
							create_board(allboards,tmp,positive,negative);
						}
						positive[k].used = false;}	
					}
					// Put the word in "right up" direction and do recursion
					if( word.size() <= board[i].size()-c && word.size() <= r+1 ){
						tmp = board;
	      				if(add_word(tmp,r,c,"ru",word,positive)){
							create_board(allboards,tmp,positive,negative);
						}
						positive[k].used = false;	
						if (rword != word){
						tmp = board;
	      				if(add_word(tmp,r,c,"ru",rword,positive)){
							create_board(allboards,tmp,positive,negative);
						}
						positive[k].used = false;	}
					}
					// Put the word in "left up" direction and do recursion
					if( word.size() <= c+1 && word.size() <= r+1  ){
						tmp = board;
	      				if(add_word(tmp,r,c,"lu",word,positive)){
							create_board(allboards,tmp,positive,negative);
						}
						positive[k].used = false;	
						if (rword != word){
						tmp = board;
	      				if(add_word(tmp,r,c,"lu",rword,positive)){
							create_board(allboards,tmp,positive,negative);
						}
						positive[k].used = false;	}
					}
					// Put the word in "down" direction and do recursion
	      			if( word.size() <= board.size()-r){
						tmp = board;
						if(add_word(tmp,r,c,"d",word,positive)){
							create_board(allboards,tmp,positive,negative);
						}
						positive[k].used = false;
						if (rword != word){
						tmp = board;
						if(add_word(tmp,r,c,"d",rword,positive)){
							create_board(allboards,tmp,positive,negative);
						}
						positive[k].used = false;}
						
					}
					// Put the word in "up" direction and do recursion
					if( word.size() <= r+1 ){ 
						tmp = board;
						if(add_word(tmp,r,c,"u",word,positive)){
							create_board(allboards,tmp,positive,negative);
						}
						positive[k].used = false;
						if (rword != word){
						tmp = board;
						if(add_word(tmp,r,c,"u",rword,positive)){
							create_board(allboards,tmp,positive,negative);
						}
						positive[k].used = false;}
					}
				}
			}
		}	
	}
	return false;
}

// Only print one solution
void print_one_board(std::ostream& out_str, std::vector<Board>& allboards){
	if(allboards.size() == 0) {			// if there is no solution
		out_str<<"No solutions found\n";
		return;
	}
	out_str << "Board:";
	for(int j = 0; j < allboards[0].size(); j++){
		out_str << "\n  ";
		for(int k = 0; k < allboards[0][j].size(); k++){
			out_str << allboards[0][j][k];
		}
	}
	out_str <<"\n";
}

// Print all the solutions
void print_all_boards(std::ostream& out_str, std::vector<Board>& allboards){
	if(allboards.size() == 0) {			// if there is no solution
		out_str <<"No solutions found\n";
		return;
	}
	out_str << allboards.size() << " solution(s)\n";
	for(int i = 0; i < allboards.size();i++){
		out_str << "Board:";
		for(int j = 0; j < allboards[i].size(); j++){
			out_str << "\n  ";
			for(int k = 0; k < allboards[i][j].size(); k++){
				out_str << allboards[i][j][k];
			}
		}
		out_str<<"\n";
	}
}

// compare two Word class in used to sort the positive words
// return true if the first word is longer then the second.
bool length(const Word& w1, const Word& w2){
	return (w1.value).size() > (w2.value).size();
}

int main(int argc, char* argv[]){
	if(argc != 4) {
		std::cerr << "Usage: " << argv[0]<< " input file\n";
		return 1;
	}
	std::ifstream in_str(argv[1]);
	if(!in_str){
		std::cout << "Could not open " << argv[1] << " to read\n";
		return 1;
	}
	std::ofstream out_str(argv[2]);
	if (!out_str){
		std::cout << "Could not open " << argv[2] <<" to write\n";
		return 1;
	}
	std::string command = argv[3];
	int row, col;
	std::string s, word;
	std::vector<std::string> negative;
	std::vector<Word> positive;
	std::vector<Board> allboards;

	in_str >> col >> row;			// read in row and column number
	while(in_str >> s && (s == "+" || s == "-")){
		in_str >> word;
		if (s == "+") positive.push_back(Word(word,false));		// add each + word into vector
		else negative.push_back(word);							// add each - word into vector
	}
	std::sort(positive.begin(), positive.end(), length);		// sort the + vector
	std::vector<char> new_row(col, ' ');						// create a new board
	Board board(row, new_row);
	create_board(allboards,board,positive,negative);			// call the function to find all boards
	if(command == "one_solution"){								// print out depends on the command line
		print_one_board(out_str, allboards);
	}
	else if(command == "all_solutions"){
		print_all_boards(out_str,allboards);
	}
	
	return 0;
}