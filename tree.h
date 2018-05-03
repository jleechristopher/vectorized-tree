// VECTORIZED TREE
// Any time we make a node, we access the data through our vector of m_nodePtrs
// If we want to access a child...
//	 if it's L, it's located at 2x; x is the index of the parent
// 	 if it's R, it's located at 2x + 1
// If our vector needs to be resized, we'll insert a bunch of nullptrs to increase the size
// 	 this way we can check if the spot is occupied/contains a node 
// NOTE: m_nodePtrs starts at 1 instead of 0 so that it can support this parent/child data access system

// NEW:
// 1) many things are const
// 2) the templated type is always passed by const ref so it is only ever copied once

// possible idea: replace all 2x with LEFT and 2x + 1 with RIGHT

#ifndef __TREE__
#define __TREE__

#include <memory>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <cmath>

template<typename T> 
class MySearchTree 
{
private:
	class Node
	{
	public:
		Node(const T& value): m_data(value) {}
		const T& getVal() const { return m_data; }

	private:
		const T m_data;
	};
    struct ValStruct
    {
        ValStruct(std::shared_ptr<Node>& ptr): m_data(ptr->getVal()), m_ptr(ptr) {}
        const T& m_data;
        std::shared_ptr<Node> m_ptr;
    };

public: 
    MySearchTree(std::function<int(T,T)> comparator = cmp): compare(comparator) 
    {
        m_nodePtrs.resize(2);
        m_nodePtrs[1] = nullptr;
    }

    // this only works with integers
    void prettyPrint()
    {
        std::cout << "\nI AM PRETTY\n";
        balance();
        int N = m_sortedVals.size();
        int h = static_cast<int>(log2(N));
        int totalLines = h + 1;
        int line = 1; 
        m_printOrder.clear();
        m_printOrder.push_back(1);
        int startingSpace = static_cast<int>(pow(2, h)) - 1;
        int charSpaces = 0; 
        // int max = m_sortedVals[m_sortedVals.size() - 1].m_data;
        // int charWidth = 0;
        // int elementWidth;
        // while (max > 0)
        // {
        //     ++charWidth;
        //     max = max/10;
        // }


        while (line <= totalLines)
        {
            for (int ii = 0; ii < startingSpace; ++ii)
            {
                std::cout << " ";
            }
            for (int index : m_printOrder)
            {
                if (exists(m_nodePtrs[index]))
                {
                    // int val = m_nodePtrs[index]->getVal();
                    // elementWidth = 0;
                    // while (val > 0)
                    // {
                    //     ++elementWidth;
                    //     val = val/10;
                    // }
                    // while (elementWidth < charWidth)
                    // {
                    //     std::cout << " ";
                    //     ++elementWidth;
                    // }
                    std::cout << m_nodePtrs[index]->getVal();
                }
                else
                {
                    std::cout << " ";
                    // for (int ii = 0; ii < charWidth; ++ii)
                    // {
                    //     std::cout << " ";
                    // }
                }
                for (int ii = 0; ii < charSpaces; ++ii)
                {
                    std::cout << " ";
                }
            }
            std::cout << "\n";

            getNextRow(m_printOrder);
            charSpaces = startingSpace;
            --h;
            startingSpace = static_cast<int>(pow(2, h)) - 1;
            ++line;
        }
    }

    int balance()
    {
        getSortedVals(1);
    	// reset tree to empty vector of size 2
    	m_nodePtrs.clear();
    	m_nodePtrs.push_back(std::shared_ptr<Node>());
    	m_nodePtrs.push_back(std::shared_ptr<Node>());

    	medianBalance(m_sortedVals, 0, m_sortedVals.size());

        return getNumBarren(1);
    }

	bool insert(const T& value)
    {
    	int pos = findIndex(value);

    	// if the spot is empty, we can insert it
        if (!static_cast<bool>(m_nodePtrs[pos]))
        {
        	m_nodePtrs[pos] = std::make_shared<MySearchTree<T>::Node>(value);
        	// balance();
        	return true; 
        }
        // if the spot is taken, it means this is a duplicate
        else
        {
        	return false;
        }
    }

	bool remove(const T& value)
    {
    	int toRemove = findIndex(value);

    	// if the spot is empty, we can't remove anything
        if (!static_cast<bool>(m_nodePtrs[toRemove]))
        {
        	return false;
        }

        // if the node has no children, we can safely remove it
        if (!hasChildren(toRemove))
        {
        	m_nodePtrs[toRemove].reset();
        	return true; 
        }

        else
        {
        	if (hasR(toRemove))
        	{
        		// if R doesn't have a left child, we need to manually swap it. Otherwise
        		// smallest() will find the smallest of R rather than toRemove
        		if (!hasL(toRemove * 2 + 1))
        		{
        			int toSwap = toRemove * 2 + 1;
	        		swap(m_nodePtrs, toRemove, toSwap); 
	        		toRemove = toSwap;        			
        		}
        		else
        		{
	        		int toSwap = smallest(toRemove * 2 + 1);
	        		swap(m_nodePtrs, toRemove, toSwap); 
	        		toRemove = toSwap;        			
        		}

        		while(hasChildren(toRemove))
        		{
        			int toSwap = smallest(toRemove);
        			swap(m_nodePtrs, toRemove, toSwap);
	        		toRemove = toSwap;
        		}

        		// at this point the node has no children, so we can delete it
        		m_nodePtrs[toRemove].reset();
        		return true;
        	}
        	else if (hasL(toRemove))
        	{
        		if (!hasR(toRemove * 2))
        		{
        			int toSwap = toRemove * 2;
	        		swap(m_nodePtrs, toRemove, toSwap); 
	        		toRemove = toSwap;        			
        		}
        		else
        		{
	        		int toSwap = largest(toRemove * 2);
	        		swap(m_nodePtrs, toRemove, toSwap); 
	        		toRemove = toSwap;        			
        		}

        		while(hasChildren(toRemove))
        		{
        			int toSwap = largest(toRemove);
        			swap(m_nodePtrs, toRemove, toSwap);
	        		toRemove = toSwap;
        		}

        		// at this point the node has no children, so we can delete it
        		m_nodePtrs[toRemove].reset();
        		return true;        		
        	}
        }  
        return false;   	
    }



	bool contains(const T& value)
    {
    	int pos = findIndex(value);

    	// if the spot is empty, our tree doesn't contain the value
        if (!static_cast<bool>(m_nodePtrs[pos]))
        {
        	return false;
        }
        // otherwise it contains the value
        else
        {
        	return true;
        }        	
    }

	MySearchTree::Node* getRoot()
    {
        return m_nodePtrs[1].get();
    }

private:
	std::vector<std::shared_ptr<Node> > m_nodePtrs;
	std::function<int(T,T)> compare;
	std::vector<ValStruct> m_sortedVals;
    std::vector<int> m_printOrder;

    void getNextRow(std::vector<int>& vec)
    {
        auto oldvec = vec;
        vec.clear();
        for (int ii : oldvec)
        {
            if (ii == 0)
            {
                vec.push_back(0);
                vec.push_back(0);
            }
            else
            {
                if (!withinCapacity(ii * 2 + 1))
                {
                    incCapacity();
                }

                if (exists(m_nodePtrs[ii * 2]))
                {
                    vec.push_back(ii * 2);
                }
                else
                {
                    vec.push_back(0);
                }
                if (exists(m_nodePtrs[ii * 2 + 1]))
                {
                    vec.push_back(ii * 2 + 1);
                }
                else
                {
                    vec.push_back(0);
                }
            }
        }
    }

    bool exists(std::shared_ptr<Node>& ptr)
    {
        return static_cast<bool>(ptr);
    }

    // Makes m_sortedVals a sorted vector of the subtree denoted by startingIndex in linear time
    // 1) goes to minimum value (leftmost node)
    // 2) checks left; if left exists and is not already in m_sortedVals it travels there
    // 3) checks current node; if current is not in m_sortedVals it adds it
    // 4) checks right; if right is not in m_sortedVals it travels there
    // 5) travels upwards. if the current node is our starting node we stop here rather than traveling upwards
    void getSortedVals(int startingIndex)
    {
        m_sortedVals.clear();
        int currentInd = startingIndex;

        // base case: wants to climb above the starting index
        while (true)
        {
            if (!withinCapacity(currentInd * 2 + 1))
            {
                incCapacity();
            }
            // check left
            if (static_cast<bool>(m_nodePtrs[currentInd * 2]))
            {
                if ((!m_sortedVals.empty()) && (compare(m_nodePtrs[currentInd * 2]->getVal(), m_sortedVals.back().m_data) <= 0))
                {
                    // ignore, since the value is already in m_sortedVals if it's less than the end element. m_sortedVals
                    // will only be empty when we're finding the leftmost node to start, and we don't want to do comparisons
                    // if it's empty. 
                }
                else
                {
                    currentInd = currentInd * 2; // go left
                    continue;
                }
            }

            // check current
            if (m_sortedVals.empty())
            {
                m_sortedVals.push_back(ValStruct(m_nodePtrs[currentInd]));
            }
            else if ( compare(m_nodePtrs[currentInd]->getVal(), m_sortedVals.back().m_data) == 1 )
            {
                m_sortedVals.push_back(ValStruct(m_nodePtrs[currentInd]));
            }

            // check right
            if (static_cast<bool>(m_nodePtrs[currentInd * 2 + 1]))
            {
                if ( compare(m_nodePtrs[currentInd * 2 + 1]->getVal(), m_sortedVals.back().m_data) == 1)
                {
                    currentInd = currentInd * 2 + 1; // go right
                    continue;
                }
                else
                {
                    // ignore
                }
            }

            // check stop condition
            if ( currentInd == startingIndex )
            {
                break;
            }
            else
            {
                currentInd = currentInd / 2; // climb up
            }
        }
    }

    // where barren = no children
    // Note: this algorithm is nearly identical to getSortedVals()
    int getNumBarren(int startingIndex)
    {
        m_sortedVals.clear();
        int currentInd = startingIndex;
        int numChildren = 0;
        int numBarren = 0;

        while (true)
        {
            if (!withinCapacity(currentInd * 2 + 1))
            {
                incCapacity();
            }
            numChildren = 0;

            // check left
            if (static_cast<bool>(m_nodePtrs[currentInd * 2]))
            {
                numChildren++;
                if ((!m_sortedVals.empty()) && (compare(m_nodePtrs[currentInd * 2]->getVal(), m_sortedVals.back().m_data) <= 0))
                {
                    // ignore, since the value is already in m_sortedVals if it's less than the end element. m_sortedVals
                    // will only be empty when we're finding the leftmost node to start, and we don't want to do comparisons
                    // if it's empty. 
                }
                else
                {
                    currentInd = currentInd * 2; // go left
                    continue;
                }
            }

            // check current
            if (m_sortedVals.empty())
            {
                m_sortedVals.push_back(ValStruct(m_nodePtrs[currentInd]));
            }
            else if ( compare(m_nodePtrs[currentInd]->getVal(), m_sortedVals.back().m_data) == 1 )
            {
                m_sortedVals.push_back(ValStruct(m_nodePtrs[currentInd]));
            }

            // check right
            if (static_cast<bool>(m_nodePtrs[currentInd * 2 + 1]))
            {
                numChildren++;
                if ( compare(m_nodePtrs[currentInd * 2 + 1]->getVal(), m_sortedVals.back().m_data) == 1)
                {
                    currentInd = currentInd * 2 + 1; // go right
                    continue;
                }
                else
                {
                    // ignore
                }
            }

            if (numChildren == 0)
            {
                // std::cout << "barren found at index " << currentInd << "\n";
                numBarren++;
            }

            // check stop condition
            if ( currentInd == startingIndex )
            {
                return numBarren;
            }
            else
            {
                currentInd = currentInd / 2; // climb up
            }
        }

    }

    void medianBalance(const std::vector<ValStruct>& vals, int beg, int end)
    {
    	// Base cases: subarray of size 0 or 1
    	if (end - beg == 1)
    	{
    		medianInsert(vals, beg);
    		return;
    	}
    	else if (end - beg == 0)
    	{
    		return;
    	}
    	else
    	{
    		int mid = beg + (end - beg) / 2;
    		medianInsert(vals, mid);
    		medianBalance(vals, beg, mid);
    		medianBalance(vals, mid+1, end);
    	}
    }

    void medianInsert(const std::vector<ValStruct>& vals, int valPos)
    {
    	int treePos = findIndex(vals[valPos].m_data);
    	m_nodePtrs[treePos] = vals[valPos].m_ptr;
    }

	int findIndex (const T& value)
	{
		int currentInd = 1; 

        // we'll continue traversing the tree until the value's location is found
        while (true)
        {
        	// if the spot is empty, return the index
        	if (!static_cast<bool>(m_nodePtrs[currentInd]))
        	{
        		return currentInd;
        	}

        	// if the value is already in the tree, return the index 
            else if (compare(value, m_nodePtrs[currentInd]->getVal()) == 0)
            {
                return currentInd;
            }

            else
            {
            	currentInd = getNext(value, currentInd);
            }   
        }
	}

	int getNext (const T& value, int currentInd)
	{
    	if (!withinCapacity(currentInd * 2 + 1))
    	{
    		incCapacity();
    	}

		if (compare(value, m_nodePtrs[currentInd]->getVal()) == 1)
		{
			return currentInd * 2 + 1; // go right
		}

		else if (compare(value, m_nodePtrs[currentInd]->getVal()) == -1)
		{
			return currentInd * 2; // go left 
		}

        return 0;
	}

    static int cmp(const T& val1,const T& val2)
    {
        if (val1 < val2) 
        {
            return -1;
        }
        else if (val1 > val2) 
        {
            return 1;
        }
        else 
        {
            return 0;
        }
    }

	int largest(int currentInd)
    {
        if ( !hasChildren(currentInd) )
        {
            throw std::invalid_argument( "Cannot find largest child of node without children" );
        }

		if (hasR(currentInd))
		{
			while (hasR(currentInd))
			{
				currentInd = currentInd * 2 + 1;
			}
			return currentInd;
		}
		else
		{
			currentInd = currentInd * 2;
			while (hasR(currentInd))
			{
				currentInd = currentInd * 2 + 1;
			}
			return currentInd;			
		}
        
    }

	int smallest(int currentInd)
    {
        if ( !hasChildren(currentInd) )
        {
            throw std::invalid_argument( "Cannot find largest child of node without children" );
        }

		if (hasL(currentInd))
		{
			while (hasL(currentInd))
			{
				currentInd = currentInd * 2;
			}
			return currentInd;
		}  
		else
		{
			currentInd = currentInd * 2 + 1;
			while (hasL(currentInd))
			{
				currentInd = currentInd * 2;
			}
			return currentInd;			
		}      
    }

    bool hasChildren(const int& currentInd)
    {
    	if (!withinCapacity(currentInd * 2 + 1))
    	{
    		incCapacity();
    	}

    	if (static_cast<bool>(m_nodePtrs[currentInd * 2]) || static_cast<bool>(m_nodePtrs[currentInd * 2 + 1]))
    	{
    		return true;
    	}
    	return false;
    }

    bool hasR(const int& currentInd)
    {
    	if (!withinCapacity(currentInd * 2 + 1))
    	{
    		incCapacity();
    	}

    	if (static_cast<bool>(m_nodePtrs[currentInd * 2 + 1]))
    	{
    		return true;
    	}
    	return false;
    }

    bool hasL(const int& currentInd)
    {
    	if (!withinCapacity(currentInd * 2 + 1))
    	{
    		incCapacity();
    	}

    	if (static_cast<bool>(m_nodePtrs[currentInd * 2]))
    	{
    		return true;
    	}
    	return false;
    }

    bool withinCapacity(const uint32_t ind) 
    {
    	if (ind < m_nodePtrs.size())
    	{
    		return true;
    	}
    	return false; 
    }

    void incCapacity()
    {
    	// double the size of our vector (and add 1) and insert a bunch of nullptrs
    	int newSize = m_nodePtrs.size() + 1;
    	for (int i = 0; i < newSize; ++i)
    	{
	    	m_nodePtrs.push_back(std::shared_ptr<Node>());
    	}
    }

	void swap(std::vector<std::shared_ptr<Node> >& vec, int lInd, int rInd)
    {
        auto temp = vec[lInd];
        vec[lInd] = vec[rInd];
        vec[rInd] = temp;
    } 

};

#endif