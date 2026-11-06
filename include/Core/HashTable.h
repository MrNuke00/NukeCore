// NukeCore\include\Core\HashTable.h
#pragma once
#include <Core/Core.h>
#include <Core/Array.h>
#include <Core/Hash.h>
#include <initializer_list>


namespace NK::Core
{	
	
	/**
	 * @brief Parent class for Map/Set, providing common functionality for hash table operations.
	 */
	template<typename K, typename V, typename Hasher, typename KeyExtr, typename KeyEqual>
	class HashTable
	{
	public:

		class Iterator
		{
		public:
			using inner = typename V*;
			Iterator() = default;
			explicit Iterator(inner It) {}
			decltype(auto) operator*()  const;
			auto operator->() const;
			Iterator& operator++();
			Iterator operator++(int);
			bool operator==(const Iterator& O) const;
			bool operator!=(const Iterator& O) const;
		private:

		};

		class ConstIterator
		{
		public:
			using inner = typename V*;
			ConstIterator() = default;
			explicit ConstIterator(inner It);
			ConstIterator(const Iterator<K, V>& It);
			decltype(auto) operator*()  const;
			auto operator->() const;
			ConstIterator& operator++();
			ConstIterator operator++(int);
			bool operator==(const ConstIterator& O) const;
			bool operator!=(const ConstIterator& O) const;
		private:

		};

	public:
		HashTable();
		HashTable(std::initializer_list<V> init);
		~HashTable();
	
	public:
		// Insert
		
		bool Insert(const V& Element, Iterator* It = nullptr);
		
		bool Insert(V&& Element, Iterator* It = nullptr);

	public:
		// Remove

		void Empty();
		
		bool Remove(const K& Key);

	public:
		// Search
		
		Iterator Find(const K& Key);
		
		ConstIterator Find(const K& Key) const;

	public:
		// Check/Info

		uint32 Num() const;
		
		bool Contains(const K& Key) const
		{ 
			return Find(Key) != end();
		}

	public:
		// Container
		
		float LoadFactor() const 
		{ 
			return m_Buckets.Num() ? (float(m_Num) / m_Buckets.Num()) : 0.f; 
		}

		uint32 BucketCount() const
		{
			return m_Buckets.Num();
		}
		
		void Reserve(uint32 Count = 1);// make sure the hash table can hold at least Count elements without rehashing
		
		void Rehash(uint32 NewBucketCount);

	public:
		// Conversion
		NK::Core::Array<V> ToArray() const;
		NK::Core::Array<K> KeyToArray() const;
		NK::Core::Array<V> ValueToArray() const;
	public:
		Iterator begin();
		Iterator end();
		ConstIterator begin() const;
		ConstIterator end() const;
	protected :
		NK::Core::Array<Array<V>> m_Buckets;
		uint32 m_Num = 0;
	};

}

