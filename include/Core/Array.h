#pragma once
#include <Core/Core.h>
#include <Core/Memory.h>

#include <initializer_list>
#include <new>
#include <assert.h>
//required : { limit } 

namespace NK::Core
{
	template<typename T>
	struct HeapAlloc
	{
	public:
		using TSize = int32;
		static constexpr TSize MaxSize = INT32_MAX;

		void* Allocate(TSize Count)
		{
			if (Count < 0) Count = 0;
			if (Count == 0) return nullptr;

			void* Ptr = Memory::Alloc<T>(Count);
			if (Ptr) m_capacity = Count;   // only on success
			return Ptr;
		}

		void Deallocate(void* Ptr)
		{
			if (Ptr) Memory::Free<T>((T*)Ptr);
			m_capacity = 0;
		}

		void Reallocate(void*& Ptr, TSize OldCount, TSize NewCount)
		{
			UNUSED(OldCount);
			if (NewCount < 0) NewCount = 0;
			if (NewCount == 0) { Deallocate(Ptr); Ptr = nullptr; return; }

			void* NewPtr = Memory::Realloc<T>(Ptr, NewCount);
			if (NewPtr) { Ptr = NewPtr; m_capacity = NewCount; }
		}

	protected:
		TSize m_capacity = 0;
	};

	template<typename T, typename TAlloc = HeapAlloc<T>>
	class Array : public TAlloc
	{
	public:
		using Alloc			= TAlloc;
		using TSize			= typename TAlloc::TSize;
		using ElementType	= T;

		template<typename U, typename A>
		friend class Array;

	private:
		T*		m_data = nullptr;
		TSize	m_size = 0;
	public:
		// Construction / Destruction
		Array() {}

		explicit Array(TSize Count)
		{
			Reserve(Count);
		}

		Array(TSize Count, const T& Value)
		{
			if (Count <= 0) return;
			Reserve(Count);
			for (TSize i = 0; i < Count; ++i)
				new (&m_data[i]) T(Value);
			m_size = Count;
		}

		Array(const T* Data, TSize Count)
		{
			Append(Data, Count);
		}

		Array(const T* First, const T* Last)
		{
			assert(Last >= First);
			Append(First, static_cast<TSize>(Last - First));
		}

		Array(std::initializer_list<T> InitList)
		{
			Append(InitList.begin(), static_cast<TSize>(InitList.size()));
		}

		Array(const Array& Other)
		{
			const TSize N = Other.m_size;
			if (N <= 0) return;
			Reserve(N);
			for (TSize i = 0; i < N; ++i)
				new (&m_data[i]) T(Other.m_data[i]);
			m_size = N;
		}

		Array(Array&& Other)
		{
			m_data = Other.m_data;
			m_size = Other.m_size;
			this->m_capacity = Other.m_capacity;

			Other.m_data = nullptr;
			Other.m_size = 0;
			Other.m_capacity = 0;
		}

		template<typename U, typename A>
		Array(const Array<U, A>& Other)
		{
			const TSize N = static_cast<TSize>(Other.Num());
			if (N <= 0) return;
			Reserve(N);
			for (TSize i = 0; i < N; ++i)
				new (&m_data[i]) T(Other[i]);
			m_size = N;
		}

		template<typename U, typename A>
		Array(Array<U, A>&& Other)
		{
			const TSize N = static_cast<TSize>(Other.Num());
			if (N > 0)
			{
				Reserve(N);
				for (TSize i = 0; i < N; ++i)
					new (&m_data[i]) T(std::move(Other[i]));
				m_size = N;
			}
			Other.Resize();
		}

		~Array() { free_data(); }
	public :

		// Assignment
		
		Array& operator=(const Array& Other)
		{
			if (this != &Other)
			{
				Resize();
				const TSize N = Other.m_size;
				if (N > 0)
				{
					grow_to(N);
					for (TSize i = 0; i < N; ++i)
					{
						new (&m_data[i]) T(Other.m_data[i]);
					}
					m_size = N;
				}
			}
			return *this;
		}

		Array& operator=(Array&& Other)
		{
			if (this != &Other)
			{
				free_data();

				m_data = Other.m_data;
				m_size = Other.m_size;
				this->m_capacity = Other.m_capacity;

				Other.m_data = nullptr;
				Other.m_size = 0;
				Other.m_capacity = 0;
			}
			return *this;
		}

		Array& operator=(std::initializer_list<T> InitList)
		{
			Resize();
			Append(InitList.begin(), static_cast<TSize>(InitList.size()));
			return *this;
		}

		template<typename U, typename A>
		Array& operator=(const Array<U, A>& Other)
		{
			Resize();
			const TSize N = static_cast<TSize>(Other.Num());
			if (N > 0)
			{
				grow_to(N);
				for (TSize i = 0; i < N; ++i)
					new (&m_data[i]) T(Other[i]);
				m_size = N;
			}
			return *this;
		}

		template<typename U, typename A>
		Array& operator=(Array<U, A>&& Other)
		{
			Resize();
			const TSize N = static_cast<TSize>(Other.Num());
			if (N > 0)
			{
				grow_to(N);
				for (TSize i = 0; i < N; ++i)
					new (&m_data[i]) T(std::move(Other[i]));
				m_size = N;
			}
			Other.Resize();
			return *this;
		}

	public :

		// Size / Capacity / Validity
		TSize	Num()	const { return m_size; }
		TSize	Max()	const { return this->m_capacity; }
		TSize	Slack()	const { return Max() - Num(); }
		bool	Empty()	const { return m_size == 0; };

		uint32 GetAllocatedSize() const { return Max() * (uint32)sizeof(T); }
		TSize  GetTypeSize()      const { return (TSize)(sizeof(T) * m_size); }

		bool IsValidIndex(TSize Idx) const { return (0 <= Idx)&&(Idx < m_size); }
		bool IsEmpty() const { return m_size == 0; }

	public :

		// Data access

		T* GetData() { return m_data; }
		const T* GetData() const { return m_data; }

		T& operator[](TSize Idx) { return m_data[Idx]; }
		const T& operator[](TSize Idx) const { return m_data[Idx]; }

		T* At(TSize Idx) { return &m_data[Idx]; }
		const T* At(TSize Idx) const { return &m_data[Idx]; }

		T& Last(TSize Idx = 0) { return m_data[m_size - 1 - Idx]; }
		const T& Last(TSize Idx = 0) const { return m_data[m_size - 1 - Idx]; }

		T& Top() { return Last(); }
		const T& Top() const { return Last(); }

		// Range-for support

		T* begin() { return m_data; }
		const T* begin() const { return m_data; }

		T* end() { return m_data + m_size; }
		const T* end()   const { return m_data + m_size; }

	public :

		// Find / Search

		bool Contains(const T& Value) const { return Find(Value) != (TSize)IndexNone; }

		template<typename PredicateType>
		bool ContainsByPredicate(PredicateType Pred) const
		{
			for (TSize i = 0; i < m_size; ++i)
				if (Pred(m_data[i])) return true;
			return false;
		}

		TSize Find(const T& Value) const
		{
			for (TSize i = 0; i < m_size; ++i)
				if (m_data[i] == Value) return i;
			return (TSize)IndexNone;
		}

		TSize FindLast(const T& Value) const
		{
			for (TSize i = m_size; i-- > 0; )
				if (m_data[i] == Value) return i;
			return (TSize)IndexNone;
		}

		template<typename KeyType>
		TSize IndexOfByKey(const KeyType& Key) const
		{
			for (TSize i = 0; i < m_size; ++i)
				if (m_data[i].Key == Key) return i;
			return (TSize)IndexNone;
		}

		template<typename PredicateType>
		TSize IndexOfByPredicate(PredicateType Pred) const
		{
			for (TSize i = 0; i < m_size; ++i)
				if (Pred(m_data[i])) return i;
			return (TSize)IndexNone;
		}

		template<typename KeyType>
		T* FindByKey(const KeyType& Key)
		{
			const TSize Idx = IndexOfByKey(Key);
			return (Idx == (TSize)IndexNone) ? nullptr : &m_data[Idx];
		}

		template<typename KeyType>
		const T* FindByKey(const KeyType& Key) const
		{
			const TSize Idx = IndexOfByKey(Key);
			return (Idx == (TSize)IndexNone) ? nullptr : &m_data[Idx];
		}

		template<typename PredicateType>
		T* FindByPredicate(PredicateType Pred)
		{
			const TSize Idx = IndexOfByPredicate(Pred);
			return (Idx == (TSize)IndexNone) ? nullptr : &m_data[Idx];
		}

		template<typename PredicateType>
		const T* FindByPredicate(PredicateType Pred) const
		{
			const TSize Idx = IndexOfByPredicate(Pred);
			return (Idx == (TSize)IndexNone) ? nullptr : &m_data[Idx];
		}

		template<typename PredicateType>
		Array FilterByPredicate(PredicateType Pred) const
		{
			Array Result;
			for (TSize i = 0; i < m_size; ++i)
				if (Pred(m_data[i])) Result.Add(m_data[i]);
			return Result;
		}

	public :

		// Resize / Reserve

		void Reserve(TSize Number)
		{
			if (Number > Max())
				resize_allocation(Number);
		}

		void SetNum(TSize NewNum, bool bShrink = true)
		{
			if (NewNum < 0) NewNum = 0;
			if (NewNum > Max()) grow_to(NewNum);

			if (NewNum < m_size)
			{
				if constexpr (!std::is_trivially_destructible_v<T>)
					for (TSize i = NewNum; i < m_size; ++i) m_data[i].~T();
			}
			else
			{
				for (TSize i = m_size; i < NewNum; ++i)
					new (&m_data[i]) T();
			}
			m_size = NewNum;

			if (bShrink) Shrink();
		}

		void SetNumZeroed(TSize NewNum, bool bShrink = true)
		{
			if (NewNum < 0) NewNum = 0;
			if (NewNum > Max()) grow_to(NewNum);

			if (NewNum < m_size)
			{
				if constexpr (!std::is_trivially_destructible_v<T>)
					for (TSize i = NewNum; i < m_size; ++i) m_data[i].~T();
			}
			else
			{
				for (TSize i = m_size; i < NewNum; ++i)
				{
					if constexpr (std::is_trivially_copyable_v<T>)
						std::memset(static_cast<void*>(m_data + i), 0, sizeof(T));
					else
						new (&m_data[i]) T{};
				}
			}
			m_size = NewNum;

			if (bShrink) Shrink();
		}

		// Only meaningful for trivially-destructible types.
		void SetNumUninitialized(TSize NewNum, bool bShrink = true)
		{
			if (NewNum < 0) NewNum = 0;
			if (NewNum > Max()) grow_to(NewNum);

			if (NewNum < m_size)
			{
				if constexpr (!std::is_trivially_destructible_v<T>)
					for (TSize i = NewNum; i < m_size; ++i) m_data[i].~T();
			}
			m_size = NewNum;   // elements in [OldSize, NewNum) stay raw

			if (bShrink) Shrink();
		}

		void Init(TSize Count, const T& Value)
		{
			if (Count < 0) Count = 0;
			Resize();
			if (Count <= 0) return;

			Reserve(Count);
			for (TSize i = 0; i < Count; ++i)
				new (&m_data[i]) T(Value);
			m_size = Count;
		}

		void Resize(TSize NewSize = 0)
		{
			destroy_all();
			if (NewSize > Max()) Reserve(NewSize);
		}

		void Empty(TSize Slack = 0)
		{
			destroy_all();
			resize_allocation(Slack > 0 ? Slack : 0);
		}

		void Shrink()
		{
			if (Max() != m_size)
				resize_allocation(m_size);
		}

	public :

		// Add

		TSize Add(const T& Value) { return insert_internal(Value, m_size); }

		TSize Add(T&& Value) { return insert_internal(std::move(Value), m_size); }

		TSize AddUnique(const T& Value)
		{
			const TSize Idx = Find(Value);
			return (Idx != (TSize)IndexNone) ? Idx : Add(Value);
		}

		TSize AddUnique(T&& Value)
		{
			const TSize Idx = Find(Value);
			return (Idx != (TSize)IndexNone) ? Idx : Add(std::move(Value));
		}

		TSize AddZeroed(TSize Count = 1)
		{
			if (Count <= 0) return m_size;
			const TSize OldSize = m_size;
			grow_to(m_size + Count);

			if constexpr (std::is_trivially_copyable_v<T>)
				std::memset(static_cast<void*>(m_data + OldSize), 0, static_cast<size_t>(Count) * sizeof(T));
			else
				for (TSize i = 0; i < Count; ++i) new (&m_data[OldSize + i]) T{};

			m_size += Count;
			return OldSize;
		}

		TSize AddDefaulted(TSize Count = 1)
		{
			if (Count <= 0) return m_size;
			const TSize OldSize = m_size;
			grow_to(m_size + Count);
			for (TSize i = 0; i < Count; ++i) new (&m_data[OldSize + i]) T();
			m_size += Count;
			return OldSize;
		}

		// Only meaningful for trivially-destructible types.
		TSize AddUninitialized(TSize Count = 1)
		{
			if (Count <= 0) return m_size;
			const TSize OldSize = m_size;
			grow_to(m_size + Count);
			m_size += Count;
			return OldSize;
		}

		TSize Append(const T* Ptr, TSize Count)
		{
			if (Count <= 0 || !Ptr) return m_size;

			const TSize OldSize = m_size;

			// Temp must stay alive until the copy loop finishes, so declare it
			// OUTSIDE the if — otherwise Ptr dangles (UB) when Temp is destroyed.
			Array Temp;
			if (aliases_storage(Ptr, Count))
			{
				Temp.Append(Ptr, Count);
				Ptr = Temp.GetData();
			}

			grow_to(m_size + Count);
			for (TSize i = 0; i < Count; ++i)
				new (&m_data[OldSize + i]) T(Ptr[i]);
			m_size += Count;
			return OldSize;
		}

		TSize Append(std::initializer_list<T> InitList)
		{
			return Append(InitList.begin(), static_cast<TSize>(InitList.size()));
		}

		TSize Append(const Array& Other)
		{
			return Append(Other.GetData(), Other.Num());
		}

	public :

		// Insert

		void Insert(const T& Value, TSize Index) { insert_internal(Value, Index); }

		void Insert(T&& Value, TSize Index) { insert_internal(std::move(Value), Index); }

		void Insert(const T* Ptr, TSize Count, TSize Index)
		{
			if (Count <= 0 || !Ptr) return;
			if (Index < 0) Index = 0;
			if (Index > m_size) Index = m_size;

			Array Temp;
			if (aliases_storage(Ptr, Count))
			{
				Temp.Append(Ptr, Count);
				Ptr = Temp.GetData();
			}

			grow_to(m_size + Count);
			shift_right(Index, Count);

			for (TSize i = 0; i < Count; ++i)
			{
				if (Index + i < m_size) m_data[Index + i].~T();   // moved-from slot
				new (&m_data[Index + i]) T(Ptr[i]);
			}
			m_size += Count;
		}

		void Insert(std::initializer_list<T> InitList, TSize Index)
		{
			Insert(InitList.begin(), static_cast<TSize>(InitList.size()), Index);
		}

		void Insert(const Array& Other, TSize Index)
		{
			Insert(Other.GetData(), Other.Num(), Index);
		}

		void InsertZeroed(TSize Index, TSize Count = 1)
		{
			if (Count <= 0) return;
			if (Index < 0) Index = 0;
			if (Index > m_size) Index = m_size;

			grow_to(m_size + Count);
			shift_right(Index, Count);

			for (TSize i = 0; i < Count; ++i)
			{
				if (Index + i < m_size) m_data[Index + i].~T();

				if constexpr (std::is_trivially_copyable_v<T>)
					std::memset(static_cast<void*>(m_data + Index + i), 0, sizeof(T));
				else
					new (&m_data[Index + i]) T{};
			}
			m_size += Count;
		}

		void InsertDefaulted(TSize Index, TSize Count = 1)
		{
			if (Count <= 0) return;
			if (Index < 0) Index = 0;
			if (Index > m_size) Index = m_size;

			grow_to(m_size + Count);
			shift_right(Index, Count);

			for (TSize i = 0; i < Count; ++i)
			{
				if (Index + i < m_size) m_data[Index + i].~T();
				new (&m_data[Index + i]) T();
			}
			m_size += Count;
		}

	public :

		// Remove

		TSize Remove(const T& Value, TSize MaxCount = 1)
		{
			if (aliases_storage(&Value, 1)) { T Copy(Value); return Remove(Copy, MaxCount); }

			TSize NumRemoved = 0;
			for (TSize i = 0; (i < m_size) && (NumRemoved < MaxCount);)
			{
				if (m_data[i] == Value) { remove_at_internal(i, 1); ++NumRemoved; }
				else ++i;
			}
			return NumRemoved;
		}

		TSize RemoveAt(TSize Index)
		{
			if (!IsValidIndex(Index)) return 0;
			remove_at_internal(Index, 1);
			return 1;
		}

		TSize RemoveAt(TSize Index, TSize Count)
		{
			if (Count <= 0 || Index < 0 || Index >= m_size) return 0;
			if (Index + Count > m_size) Count = m_size - Index;
			remove_at_internal(Index, Count);
			return Count;
		}

		TSize RemoveAll(const T& Value)
		{
			if (aliases_storage(&Value, 1)) { T Copy(Value); return RemoveAll(Copy); }

			TSize NumRemoved = 0;
			for (TSize i = 0; i < m_size; )
			{
				if (m_data[i] == Value) { remove_at_internal(i, 1); ++NumRemoved; }
				else ++i;
			}
			return NumRemoved;
		}

		template<typename PredicateType>
		TSize RemoveAll(const PredicateType& Pred, bool bAllowShrinking = true)
		{
			TSize NumRemoved = 0;
			for (TSize i = 0; i < m_size; )
			{
				if (Pred(m_data[i])) { remove_at_internal(i, 1); ++NumRemoved; }
				else ++i;
			}
			if (bAllowShrinking && NumRemoved > 0) Shrink();
			return NumRemoved;
		}

		T Pop(bool bAllowShrinking = false)
		{
			assert(m_size > 0);
			T Result(std::move(m_data[m_size - 1]));
			m_data[m_size - 1].~T();
			--m_size;
			if (bAllowShrinking) Shrink();
			return Result;
		}

	private:
		
		// Internals
		
		static void destroy_range(T* Data, TSize Count)
		{
			if constexpr (!std::is_trivially_destructible_v<T>)
				for (TSize i = 0; i < Count; ++i) Data[i].~T();
		}

		void destroy_all()
		{
			destroy_range(m_data, m_size);
			m_size = 0;
		}

		void free_data()
		{
			destroy_all();
			if (m_data)
			{
				Alloc::Deallocate(m_data);
				m_data = nullptr;
			}
		}

		bool aliases_storage(const T* Ptr, TSize Count) const
		{
			if (!m_data || !Ptr || Count <= 0) return false;

			const T* Begin = Ptr;
			const T* End = Ptr + Count;
			const T* MyBegin = m_data;
			const T* MyEnd = m_data + m_size;

			// std::less => well-defined even for unrelated pointers
			return std::less<const T*>()(Begin, MyEnd) &&
				std::less<const T*>()(MyBegin, End);
		}

		// Grow with geometric policy, never past MaxSize.
		void grow_to(TSize Required)
		{
			if (Required <= Max()) return;

			TSize NewCap = (Max() > 0) ? Max() : 4;
			while (NewCap < Required)
			{
				if (NewCap > Alloc::MaxSize / 2) { NewCap = Required; break; }
				NewCap *= 2;
			}
			if (NewCap > Alloc::MaxSize) NewCap = Alloc::MaxSize;

			resize_allocation(NewCap);
		}

		// Single point of raw-memory management. Moves live elements,
		// destroys anything that falls outside the new capacity.
		void resize_allocation(TSize NewCapacity)
		{
			if (NewCapacity < 0) NewCapacity = 0;

			const TSize OldCapacity = Max();
			if (NewCapacity == OldCapacity) return;

			if (NewCapacity == 0)
			{
				if (m_data)
				{
					Alloc::Deallocate(m_data);
					m_data = nullptr;
				}
				m_size = 0;
				this->m_capacity = 0; // <--- Explicitly reset capacity
				return;
			}

			void* NewRaw = Alloc::Allocate(NewCapacity);
			if (!NewRaw) return; // OOM: keep old state

			T* NewData = static_cast<T*>(NewRaw);
			const TSize Keep = (m_size < NewCapacity) ? m_size : NewCapacity;

			if (m_data)
			{
				if constexpr (std::is_trivially_copyable_v<T>)
				{
					if (Keep > 0)
						std::memcpy(NewData, m_data, static_cast<size_t>(Keep) * sizeof(T));
				}
				else
				{
					for (TSize i = 0; i < Keep; ++i)
					{
						new (&NewData[i]) T(std::move(m_data[i]));
						m_data[i].~T();
					}
					for (TSize i = Keep; i < m_size; ++i)
						m_data[i].~T();
				}

				Alloc::Deallocate(m_data);
			}

			m_data = NewData;
			m_size = Keep;
			this->m_capacity = NewCapacity;
		}

		// Requires: Max() >= m_size + Count. Leaves [Index, Index+Count) as
		// moved-from (non-trivial) or duplicate bytes (trivial) slots.
		void shift_right(TSize Index, TSize Count)
		{
			const TSize NumToMove = m_size - Index;
			if (NumToMove <= 0 || Count <= 0) return;

			if constexpr (std::is_trivially_copyable_v<T>)
			{
				std::memmove(m_data + Index + Count, m_data + Index,
					static_cast<size_t>(NumToMove) * sizeof(T));
			}
			else
			{
				TSize Src = m_size;
				TSize Dst = m_size + Count;
				while (Src > Index)
				{
					--Src; --Dst;
					if (Dst >= m_size) // raw memory
						new (&m_data[Dst]) T(std::move(m_data[Src]));
					else
						m_data[Dst] = std::move(m_data[Src]);
				}
			}
		}

		void remove_at_internal(TSize Index, TSize Count)
		{
			const TSize NumToMove = m_size - Index - Count;

			if constexpr (std::is_trivially_copyable_v<T>)
			{
				if (NumToMove > 0)
					std::memmove(m_data + Index, m_data + Index + Count,
						static_cast<size_t>(NumToMove) * sizeof(T));
			}
			else
			{
				// Move-assign into the still-alive slots, then destroy the tail.
				for (TSize i = 0; i < NumToMove; ++i)
					m_data[Index + i] = std::move(m_data[Index + Count + i]);

				for (TSize i = m_size - Count; i < m_size; ++i)
					m_data[i].~T();
			}

			m_size -= Count;
		}

		template<typename U>
		TSize insert_internal(U&& Value, TSize Index)
		{
			if (Index < 0) Index = 0;
			if (Index > m_size) Index = m_size;

			// Materialize before the (possible) reallocation so Value may alias
			// an element of this array.
			T Temp(std::forward<U>(Value));

			grow_to(m_size + 1);
			shift_right(Index, 1);

			if (Index < m_size) m_data[Index].~T();   // moved-from slot
			new (&m_data[Index]) T(std::move(Temp));

			++m_size;
			return Index;
		}
	};

}

