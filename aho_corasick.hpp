#ifndef BOOST_ALGORITHM_AHO_CORASICK_SEARCH_HPP
#define BOOST_ALGORITHM_AHO_CORASICK_SEARCH_HPP

#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<vector>
#include<queue>
#include<memory>

#define MAX 256

/* References:
   http://www.cs.uku.fi/~kilpelai/BSA05/lectures/slides04.pdf
   http://en.wikipedia.org/wiki/Aho%E2%80%93Corasick_string_matching_algorithm
   Note: compile with -std=c++11

*/
    template <typename patContainerIter, typename patIter> 
    class aho_corasick {
    public:
	struct trie_node {
	    std::shared_ptr<trie_node> children[MAX];
	    std::shared_ptr<trie_node> failure_node;
	    std::vector<std::pair<patIter, patIter> > output;
	};
	std::shared_ptr<trie_node> state;

	aho_corasick(patContainerIter first, patContainerIter last): pat_first(first), pat_last(last), root(std::shared_ptr<trie_node>(new trie_node)) {
	    state = root;
	    gotoFn(root);
	    failureFn(root);
	}
	~aho_corasick () {};

        /// \fn operator ( corpusIter corpus_first, corpusIter corpus_last, std::vector<std::pair<patIter, corpusIter> > &output_vec, bool continued = true )
        /// \brief Searches the corpus for the pattern that was passed into the constructor
        /// 
        /// \param corpus_first The start of the data to search (Random Access Iterator)
        /// \param corpus_last  One past the end of the data to search
        /// \param output_vec The container which stores the matches as a vector of pairs of the iterator to the pattern and an iterator to the end of the pattern string found in the text.(The final output)
	/// \param continued A variable which stores whether or not the user wants to continue searching and retain the state so that the search can continue from where it was left off.
	template <typename corpusIter> 	
	void operator ()(corpusIter corpus_first, corpusIter corpus_last, std::vector<std::pair<patIter, corpusIter> > &output_vec, bool continued = true) {
	    this->search(corpus_first, corpus_last, output_vec, continued);
	}

    private:
	typedef typename std::iterator_traits<patContainerIter>::difference_type difference_type;
	std::shared_ptr<trie_node> root;
	patContainerIter pat_first, pat_last;
	
	/* internal function for building the trie*/
	void enter (patIter p_first, patIter p_last, std::shared_ptr<trie_node> root) {
	    std::shared_ptr<trie_node> temp = root;
	    int i = 0;
	    const int m = std::distance( p_first, p_last );
	    while(temp -> children[p_first[i]]) {
		temp = temp -> children[p_first[i]];
		i++;
	    }
	
	    for(int j = i; j< m; j++) {
		temp -> children[p_first[j]] = std::shared_ptr<trie_node>(new trie_node());
		temp = temp -> children[p_first[j]];
	    }
	    temp -> output.push_back(std::make_pair(p_first, p_last));
	} 
	
	/* builds the trie by adding the pattern strings */
	void gotoFn (std::shared_ptr<trie_node> root) {
	    const int n = std::distance(pat_first, pat_last);
	    for(int iter = 0; iter < n; iter++) {
		auto str = pat_first[iter];
		enter( str.begin(), str.end(), root);
	    }
			
	    for(int i = 0; i < MAX; i++) {
		if (!(root -> children[i]))
			root -> children[i] = root;
	    }
	}

	/* finds and adds the failure nodes to the trie */
	void failureFn(std::shared_ptr <trie_node> root) {
	    std::queue <std::shared_ptr<trie_node> > nodes_queue;
	    for(int i = 0; i < MAX; i++) {
		if(root -> children[i] != root ) {
			root -> children[i] -> failure_node = root;
			nodes_queue.push(root -> children[i]);
		}
	    }
	    std::shared_ptr<trie_node>curr_node;
	    while(!nodes_queue.empty()) {
		curr_node = nodes_queue.front();
		nodes_queue.pop();
		for(int i = 0; i < MAX; i++) {
		    if(curr_node -> children[i]) {
			std::shared_ptr<trie_node>  curr_node_child = curr_node -> children[i];
			std::shared_ptr<trie_node>  temp_node = curr_node;
			nodes_queue.push(curr_node_child);
			temp_node = temp_node -> failure_node;
			if( !(temp_node -> children[i])) {
			    while(!(temp_node -> children[i])) 
			        temp_node = temp_node -> failure_node;
			}
			curr_node_child -> failure_node = temp_node -> children[i];
			int size = temp_node -> children[i] -> output.size();
			for(int j = 0; j < size; j++)
			    curr_node_child -> output.push_back(temp_node -> children[i] -> output[j]);
		    }
		}
	    }
	}

	/* searches the trie for possible matches */
	template <typename corpusIter> 
	void search(corpusIter corpus_first, corpusIter corpus_last, std::vector<std::pair<patIter, corpusIter> > &output_vec, bool continued = true ) {
	    const int corpus_length = std::distance(corpus_first, corpus_last);
	    corpusIter curPos = corpus_first;
	    if (continued == false)
		state = root;
	    for(int i = 0; i < corpus_length; i++) {
	        while( !(state-> children[corpus_first[i]]))
		    state = state -> failure_node;
		
		state = state -> children[corpus_first[i]];

		if( !(state -> output.empty())  ) {
		    for(int i = 0; i< state -> output.size(); i++)
		        output_vec.push_back(std::make_pair(std::get<0>(state->output[i]), curPos));
		}
		curPos++;
	    }
	}

    };
    template <typename patContainerIter, typename patIter, typename corpusIter>
    void aho_corasick_search ( corpusIter corpus_first, corpusIter corpus_last, 
                  patContainerIter pat_first, patContainerIter pat_last, std::vector<std::pair<patIter, corpusIter> > &output_vec, bool continued = true) {
	aho_corasick<patContainerIter, corpusIter> obj(pat_first, pat_last );
	obj(corpus_first, corpus_last, output_vec);
    }
	
#endif 
