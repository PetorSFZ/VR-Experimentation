// Copyright (c) Peter Hillerstr�m (skipifzero.com, peter@hstroem.se)
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

namespace sfz {

// HashMap (implementation): Constructors & destructors
// ------------------------------------------------------------------------------------------------

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
HashMap<K,V,Hash,KeyEqual,Allocator>::HashMap(uint32_t suggestedCapacity) noexcept
{
	this->rehash(suggestedCapacity);
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
HashMap<K,V,Hash,KeyEqual,Allocator>::HashMap(const HashMap& other) noexcept
{
	*this = other;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
HashMap<K,V,Hash,KeyEqual,Allocator>& HashMap<K,V,Hash,KeyEqual,Allocator>::operator= (const HashMap& other) noexcept
{
	// Clear and rehash this HashMap
	this->clear();
	this->rehash(other.mCapacity);

	// Add all elements from other HashMap
	for (ConstKeyValuePair pair : other) {
		this->put(pair.key, pair.value);
	}

	return *this;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
HashMap<K,V,Hash,KeyEqual,Allocator>::HashMap(HashMap&& other) noexcept
{
	this->swap(other);
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
HashMap<K,V,Hash,KeyEqual,Allocator>& HashMap<K,V,Hash,KeyEqual,Allocator>::operator= (HashMap&& other) noexcept
{
	this->swap(other);
	return *this;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
HashMap<K,V,Hash,KeyEqual,Allocator>::~HashMap() noexcept
{
	this->destroy();
}

// HashMap (implementation): Getters
// ------------------------------------------------------------------------------------------------

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
V* HashMap<K,V,Hash,KeyEqual,Allocator>::get(const K& key) noexcept
{
	// Finds the index of the element
	uint32_t firstFreeSlot = uint32_t(~0);
	bool elementFound = false;
	bool isPlaceholder = false;
	uint32_t index = this->findElementIndex(key, elementFound, firstFreeSlot, isPlaceholder);

	// Returns nullptr if map doesn't contain element
	if (!elementFound) return nullptr;

	// Returns pointer to element
	return &(valuesPtr()[index]);
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
const V* HashMap<K,V,Hash,KeyEqual,Allocator>::get(const K& key) const noexcept
{
	// Finds the index of the element
	uint32_t firstFreeSlot = uint32_t(~0);
	bool elementFound = false;
	bool isPlaceholder = false;
	uint32_t index = this->findElementIndex(key, elementFound, firstFreeSlot, isPlaceholder);

	// Returns nullptr if map doesn't contain element
	if (!elementFound) return nullptr;

	// Returns pointer to element
	return &(valuesPtr()[index]);
}

// HashMap (implementation): Public methods
// ------------------------------------------------------------------------------------------------

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
void HashMap<K,V,Hash,KeyEqual,Allocator>::put(const K& key, const V& value) noexcept
{
	ensureProperlyHashed();

	// Finds the index of the element
	uint32_t firstFreeSlot = uint32_t(~0);
	bool elementFound = false;
	bool isPlaceholder = false;
	uint32_t index = this->findElementIndex(key, elementFound, firstFreeSlot, isPlaceholder);

	// If map contains key just replace value and return
	if (elementFound) {
		valuesPtr()[index] = value;
		return;
	}

	// Otherwise insert info, key and value
	setElementInfo(firstFreeSlot, ELEMENT_INFO_OCCUPIED);
	new (keysPtr() + firstFreeSlot) K(key);
	new (valuesPtr() + firstFreeSlot) V(value);

	mSize += 1;
	if (isPlaceholder) mPlaceholders -= 1;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
void HashMap<K,V,Hash,KeyEqual,Allocator>::put(const K& key, V&& value) noexcept
{
	ensureProperlyHashed();

	// Finds the index of the element
	uint32_t firstFreeSlot = uint32_t(~0);
	bool elementFound = false;
	bool isPlaceholder = false;
	uint32_t index = this->findElementIndex(key, elementFound, firstFreeSlot, isPlaceholder);

	// If map contains key just replace value and return
	if (elementFound) {
		valuesPtr()[index] = std::move(value);
		return;
	}

	// Otherwise insert info, key and value
	setElementInfo(firstFreeSlot, ELEMENT_INFO_OCCUPIED);
	new (keysPtr() + firstFreeSlot) K(key);
	new (valuesPtr() + firstFreeSlot) V(std::move(value));

	mSize += 1;
	if (isPlaceholder) mPlaceholders -= 1;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
V& HashMap<K,V,Hash,KeyEqual,Allocator>::operator[] (const K& key) noexcept
{
	if (mCapacity == 0) ensureProperlyHashed();

	// Finds the index of the element
	uint32_t firstFreeSlot = uint32_t(~0);
	bool elementFound = false;
	bool isPlaceholder = false;
	uint32_t index = this->findElementIndex(key, elementFound, firstFreeSlot, isPlaceholder);

	// Check if HashMap needs to rehashed
	if (!elementFound) {
		if (ensureProperlyHashed()) {
			// If rehashed redo the search so we have valid indices
			index = this->findElementIndex(key, elementFound, firstFreeSlot, isPlaceholder);
		}
	}

	// If element doesn't exist create it with default constructor
	if (!elementFound) {
		index = firstFreeSlot;
		mSize += 1;
		if (isPlaceholder) mPlaceholders -= 1;

		// Otherwise insert info, key and value
		setElementInfo(index, ELEMENT_INFO_OCCUPIED);
		new (keysPtr() + index) K(key);
		new (valuesPtr() + index) V();
	}

	// Returns pointer to element
	return valuesPtr()[index];
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
bool HashMap<K,V,Hash,KeyEqual,Allocator>::remove(const K& key) noexcept
{
	// Finds the index of the element
	uint32_t firstFreeSlot = uint32_t(~0);
	bool elementFound = false;
	bool isPlaceholder = false;
	uint32_t index = this->findElementIndex(key, elementFound, firstFreeSlot, isPlaceholder);

	// Returns nullptr if map doesn't contain element
	if (!elementFound) return false;

	// Remove element
	setElementInfo(index, ELEMENT_INFO_PLACEHOLDER);
	keysPtr()[index].~K();
	valuesPtr()[index].~V();

	mSize -= 1;
	mPlaceholders += 1;
	return true;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
void HashMap<K,V,Hash,KeyEqual,Allocator>::swap(HashMap& other) noexcept
{
	uint32_t thisSize = this->mSize;
	uint32_t thisCapacity = this->mCapacity;
	uint32_t thisPlaceholders = this->mPlaceholders;
	uint8_t* thisDataPtr = this->mDataPtr;

	this->mSize = other.mSize;
	this->mCapacity = other.mCapacity;
	this->mPlaceholders = other.mPlaceholders;
	this->mDataPtr = other.mDataPtr;

	other.mSize = thisSize;
	other.mCapacity = thisCapacity;
	other.mPlaceholders = thisPlaceholders;
	other.mDataPtr = thisDataPtr;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
void HashMap<K,V,Hash,KeyEqual,Allocator>::rehash(uint32_t suggestedCapacity) noexcept
{
	if (suggestedCapacity < mCapacity) suggestedCapacity = mCapacity;
	if (suggestedCapacity == 0) return;

	// Convert the suggested capacity to a larger (if possible) prime number
	uint32_t newCapacity = findPrimeCapacity(suggestedCapacity);

	// Create a new HashMap and allocate memory to it
	HashMap tmp;
	tmp.mCapacity = newCapacity;
	tmp.mDataPtr = static_cast<uint8_t*>(Allocator::allocate(tmp.sizeOfAllocatedMemory(), ALIGNMENT));
	std::memset(tmp.mDataPtr, 0, tmp.sizeOfAllocatedMemory());

	// Iterate over all pairs of objects in this HashMap and move them to the new one
	if (this->mDataPtr != nullptr) {
		for (KeyValuePair pair : *this) {
			tmp.put(pair.key, std::move(pair.value));
		}
	}

	// Replace this HashMap with the new one
	this->swap(tmp);
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
bool HashMap<K,V,Hash,KeyEqual,Allocator>::ensureProperlyHashed() noexcept
{
	// If HashMap is empty initialize with smallest size
	if (mCapacity == 0) {
		this->rehash(1);
		return true;
	}

	// Check if HashMap needs to be rehashed
	uint32_t maxOccupied = uint32_t(MAX_OCCUPIED_REHASH_FACTOR * mCapacity);
	if ((mSize + mPlaceholders) > maxOccupied) {

		// Determine whether capacity needs to be increased or not.
		uint32_t newCapacity = mCapacity;
		uint32_t maxSize = uint32_t(MAX_OCCUPIED_REHASH_FACTOR * mCapacity);
		if (mSize > maxSize) {
			mCapacity += 1;
		}

		this->rehash(newCapacity);
		return true;
	}

	return false;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
void HashMap<K,V,Hash,KeyEqual,Allocator>::clear() noexcept
{
	if (mSize == 0) return;

	// Call destructor for all active keys and values if they are not trivially destructible
	if (!std::is_trivially_destructible<K>::value && !std::is_trivially_destructible<V>::value) {
		K* keyPtr = keysPtr();
		V* valuePtr = valuesPtr();
		for (size_t i = 0; i < mCapacity; ++i) {
			if (elementInfo((uint32_t)i) == ELEMENT_INFO_OCCUPIED) {
				keyPtr[i].~K();
				valuePtr[i].~V();
			}
		}
	}
	else if (!std::is_trivially_destructible<K>::value) {
		K* keyPtr = keysPtr();
		for (size_t i = 0; i < mCapacity; ++i) {
			if (elementInfo((uint32_t)i) == ELEMENT_INFO_OCCUPIED) {
				keyPtr[i].~K();
			}
		}
	}
	else if (!std::is_trivially_destructible<V>::value) {
		V* valuePtr = valuesPtr();
		for (size_t i = 0; i < mCapacity; ++i) {
			if (elementInfo((uint32_t)i) == ELEMENT_INFO_OCCUPIED) {
				valuePtr[i].~V();
			}
		}
	}

	// Clear all element info bits
	std::memset(elementInfoPtr(), 0, sizeOfElementInfoArray());

	// Set size to 0
	mSize = 0;
	mPlaceholders = 0;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
void HashMap<K,V,Hash,KeyEqual,Allocator>::destroy() noexcept
{
	if (mDataPtr == nullptr) return;

	// Remove elements
	this->clear();

	// Deallocate memory
	Allocator::deallocate(mDataPtr);
	mCapacity = 0;
	mPlaceholders = 0;
	mDataPtr = nullptr;
}

// HashMap (implementation): Iterators
// ------------------------------------------------------------------------------------------------

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
typename HashMap<K,V,Hash,KeyEqual,Allocator>::Iterator& HashMap<K,V,Hash,KeyEqual,Allocator>::Iterator::operator++ () noexcept
{
	// Go through map until we find next occupied slot
	for (uint32_t i = mIndex + 1; i < mHashMap->mCapacity; ++i) {
		uint8_t info = mHashMap->elementInfo(i);
		if (info == ELEMENT_INFO_OCCUPIED) {
			mIndex = i;
			return *this;
		}
	}

	// Did not find any more elements, set to end
	mIndex = uint32_t(~0);
	return *this;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
typename HashMap<K,V,Hash,KeyEqual,Allocator>::Iterator HashMap<K,V,Hash,KeyEqual,Allocator>::Iterator::operator++ (int) noexcept
{
	auto copy = *this;
	++(*this);
	return copy;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
typename HashMap<K,V,Hash,KeyEqual,Allocator>::KeyValuePair HashMap<K,V,Hash,KeyEqual,Allocator>::Iterator::operator* () noexcept
{
	sfz_assert_debug(mIndex != uint32_t(~0));
	sfz_assert_debug(mHashMap->elementInfo(mIndex) == ELEMENT_INFO_OCCUPIED);
	return KeyValuePair(mHashMap->keysPtr()[mIndex], mHashMap->valuesPtr()[mIndex]);
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
bool HashMap<K,V,Hash,KeyEqual,Allocator>::Iterator::operator== (const Iterator& other) const noexcept
{
	return (this->mHashMap == other.mHashMap) && (this->mIndex == other.mIndex);
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
bool HashMap<K,V,Hash,KeyEqual,Allocator>::Iterator::operator!= (const Iterator& other) const noexcept
{
	return !(*this == other);
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
typename HashMap<K,V,Hash,KeyEqual,Allocator>::ConstIterator& HashMap<K,V,Hash,KeyEqual,Allocator>::ConstIterator::operator++ () noexcept
{
	// Go through map until we find next occupied slot
	for (uint32_t i = mIndex + 1; i < mHashMap->mCapacity; ++i) {
		uint8_t info = mHashMap->elementInfo(i);
		if (info == ELEMENT_INFO_OCCUPIED) {
			mIndex = i;
			return *this;
		}
	}

	// Did not find any more elements, set to end
	mIndex = uint32_t(~0);
	return *this;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
typename HashMap<K,V,Hash,KeyEqual,Allocator>::ConstIterator HashMap<K,V,Hash,KeyEqual,Allocator>::ConstIterator::operator++ (int) noexcept
{
	auto copy = *this;
	++(*this);
	return copy;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
typename HashMap<K,V,Hash,KeyEqual,Allocator>::ConstKeyValuePair HashMap<K,V,Hash,KeyEqual,Allocator>::ConstIterator::operator* () noexcept
{
	sfz_assert_debug(mIndex != uint32_t(~0));
	sfz_assert_debug(mHashMap->elementInfo(mIndex) == ELEMENT_INFO_OCCUPIED);
	return ConstKeyValuePair(mHashMap->keysPtr()[mIndex], mHashMap->valuesPtr()[mIndex]);
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
bool HashMap<K,V,Hash,KeyEqual,Allocator>::ConstIterator::operator== (const ConstIterator& other) const noexcept
{
	return (this->mHashMap == other.mHashMap) && (this->mIndex == other.mIndex);
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
bool HashMap<K,V,Hash,KeyEqual,Allocator>::ConstIterator::operator!= (const ConstIterator& other) const noexcept
{
	return !(*this == other);
}

// HashMap (implementation): Iterator methods
// ------------------------------------------------------------------------------------------------

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
typename HashMap<K,V,Hash,KeyEqual,Allocator>::Iterator HashMap<K,V,Hash,KeyEqual,Allocator>::begin() noexcept
{
	if (this->size() == 0) return Iterator(*this, uint32_t(~0));
	Iterator it(*this, 0);
	// Unless there happens to be an element in slot 0 we increment the iterator to find it
	if (elementInfo(uint32_t(0)) != ELEMENT_INFO_OCCUPIED) {
		++it;
	}
	return it;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
typename HashMap<K,V,Hash,KeyEqual,Allocator>::ConstIterator HashMap<K,V,Hash,KeyEqual,Allocator>::begin() const noexcept
{
	return cbegin();
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
typename HashMap<K,V,Hash,KeyEqual,Allocator>::ConstIterator HashMap<K,V,Hash,KeyEqual,Allocator>::cbegin() const noexcept
{
	if (this->size() == 0) return ConstIterator(*this, uint32_t(~0));
	ConstIterator it(*this, 0);
	// Unless there happens to be an element in slot 0 we increment the iterator to find it
	if (elementInfo(uint32_t(0)) != ELEMENT_INFO_OCCUPIED) {
		++it;
	}
	return it;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
typename HashMap<K,V,Hash,KeyEqual,Allocator>::Iterator HashMap<K,V,Hash,KeyEqual,Allocator>::end() noexcept
{
	return Iterator(*this, uint32_t(~0));
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
typename HashMap<K,V,Hash,KeyEqual,Allocator>::ConstIterator HashMap<K,V,Hash,KeyEqual,Allocator>::end() const noexcept
{
	return cend();
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
typename HashMap<K,V,Hash,KeyEqual,Allocator>::ConstIterator HashMap<K,V,Hash,KeyEqual,Allocator>::cend() const noexcept
{
	return ConstIterator(*this, uint32_t(~0));
}

// HashMap (implementation): Private methods
// ------------------------------------------------------------------------------------------------

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
uint32_t HashMap<K,V,Hash,KeyEqual,Allocator>::findPrimeCapacity(uint32_t capacity) const noexcept
{
	constexpr uint32_t PRIMES[] = {
		67,
		131,
		257,
		521,
		1031,
		2053,
		4099,
		8209,
		16411,
		32771,
		65537,
		131101,
		262147,
		524309,
		1048583,
		2097169,
		4194319,
		8388617,
		16777259,
		33554467,
		67108879,
		134217757,
		268435459,
		536870923,
		1073741827,
		2147483659
	};

	// Linear search is probably okay for an array this small
	for (size_t i = 0; i < sizeof(PRIMES) / sizeof(uint32_t); ++i) {
		if (PRIMES[i] >= capacity) return PRIMES[i];
	}

	// Found no prime, which means that the suggested capacity is too large.
	return MAX_CAPACITY;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
size_t HashMap<K,V,Hash,KeyEqual,Allocator>::sizeOfElementInfoArray() const noexcept
{
	// 2 bits per info element, + 1 since mCapacity always is odd.
	size_t infoMinRequiredSize = (mCapacity >> 2) + 1;

	// Calculate how many alignment sized chunks is needed to store element info
	size_t infoNumAlignmentSizedChunks = (infoMinRequiredSize >> ALIGNMENT_EXP) + 1;
	return infoNumAlignmentSizedChunks << ALIGNMENT_EXP;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
size_t HashMap<K,V,Hash,KeyEqual,Allocator>::sizeOfKeyArray() const noexcept
{
	// Calculate how many aligment sized chunks is needed to store keys
	size_t keysMinRequiredSize = mCapacity * sizeof(K);
	size_t keyNumAlignmentSizedChunks = (keysMinRequiredSize >> ALIGNMENT_EXP) + 1;
	return keyNumAlignmentSizedChunks << ALIGNMENT_EXP;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
size_t HashMap<K,V,Hash,KeyEqual,Allocator>::sizeOfValueArray() const noexcept
{
	// Calculate how many alignment sized chunks is needed to store values
	size_t valuesMinRequiredSize = mCapacity * sizeof(V);
	size_t valuesNumAlignmentSizedChunks = (valuesMinRequiredSize >> ALIGNMENT_EXP) + 1;
	return valuesNumAlignmentSizedChunks << ALIGNMENT_EXP;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
size_t HashMap<K,V,Hash,KeyEqual,Allocator>::sizeOfAllocatedMemory() const noexcept
{
	return sizeOfElementInfoArray() + sizeOfKeyArray() + sizeOfValueArray();
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
uint8_t* HashMap<K,V,Hash,KeyEqual,Allocator>::elementInfoPtr() const noexcept
{
	return mDataPtr;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
K* HashMap<K,V,Hash,KeyEqual,Allocator>::keysPtr() const noexcept
{
	return reinterpret_cast<K*>(mDataPtr + sizeOfElementInfoArray());
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
V* HashMap<K,V,Hash,KeyEqual,Allocator>::valuesPtr() const noexcept
{
	return reinterpret_cast<V*>(mDataPtr + sizeOfElementInfoArray() + sizeOfKeyArray());
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
uint8_t HashMap<K,V,Hash,KeyEqual,Allocator>::elementInfo(uint32_t index) const noexcept
{
	uint32_t chunkIndex = index >> 2; // index / 4;
	uint32_t chunkIndexModulo = index & 0x03; // index % 4
	uint32_t chunkIndexModuloTimes2 = chunkIndexModulo << 1;

	uint8_t chunk = elementInfoPtr()[chunkIndex];
	uint8_t info = static_cast<uint8_t>((chunk >> (chunkIndexModuloTimes2)) & 0x3);

	return info;
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
void HashMap<K,V,Hash,KeyEqual,Allocator>::setElementInfo(uint32_t index, uint8_t value) noexcept
{
	uint32_t chunkIndex = index >> 2; // index / 4;
	uint32_t chunkIndexModulo = index & 0x03; // index % 4
	uint32_t chunkIndexModuloTimes2 = chunkIndexModulo << 1;

	uint8_t chunk = elementInfoPtr()[chunkIndex];

	// Remove previous info
	chunk = chunk & (~(uint32_t(0x03) << chunkIndexModuloTimes2));

	// Insert new info
	elementInfoPtr()[chunkIndex] =  uint8_t(chunk | (value << chunkIndexModuloTimes2));
}

template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
uint32_t HashMap<K,V,Hash,KeyEqual,Allocator>::findElementIndex(const K& key, bool& elementFound, uint32_t& firstFreeSlot, bool& isPlaceholder) const noexcept
{
	Hash keyHasher;
	KeyEqual keyComparer;

	elementFound = false;
	firstFreeSlot = uint32_t(~0);
	isPlaceholder = false;
	K* const keys = keysPtr();

	// Hash the key and find the base index
	const int64_t baseIndex = int64_t(keyHasher(key) % size_t(mCapacity));

	// Check if base index holds the element
	uint8_t info = elementInfo(uint32_t(baseIndex));
	if (info == ELEMENT_INFO_EMPTY) {
		firstFreeSlot = uint32_t(baseIndex);
		return uint32_t(~0);
	}
	else if (info == ELEMENT_INFO_PLACEHOLDER) {
		firstFreeSlot = uint32_t(baseIndex);
		isPlaceholder = true;
	}
	else if (info == ELEMENT_INFO_OCCUPIED) {
		if (keyComparer(keys[baseIndex], key)) {
			elementFound = true;
			return uint32_t(baseIndex);
		}
	}

	// Search for the element using quadratic probing
	const int64_t maxNumProbingAttempts = int64_t(mCapacity);
	for (int64_t i = 1; i < maxNumProbingAttempts; ++i) {
		const int64_t iSquared = i * i;
		
		// Try (base + i�) index
		int64_t index = (baseIndex + iSquared) % int64_t(mCapacity);
		info = elementInfo(uint32_t(index));
		if (info == ELEMENT_INFO_EMPTY) {
			if (firstFreeSlot == uint32_t(~0)) firstFreeSlot = uint32_t(index);
			break;
		} else if (info == ELEMENT_INFO_PLACEHOLDER) {
			if (firstFreeSlot == uint32_t(~0)) {
				firstFreeSlot = uint32_t(index);
				isPlaceholder = true;
			}
		} else if (info == ELEMENT_INFO_OCCUPIED) {
			if (keyComparer(keys[index], key)) {
				elementFound = true;
				return uint32_t(index);
			}
		}

		// Try (base - i�) index
		index = (((baseIndex - iSquared) % int64_t(mCapacity)) + int64_t(mCapacity)) % int64_t(mCapacity);
		info = elementInfo(uint32_t(index));
		if (info == ELEMENT_INFO_EMPTY) {
			if (firstFreeSlot == uint32_t(~0)) firstFreeSlot = uint32_t(index);
			break;
		} else if (info == ELEMENT_INFO_PLACEHOLDER) {
			if (firstFreeSlot == uint32_t(~0)) {
				firstFreeSlot = uint32_t(index);
				isPlaceholder = true;
			}
		} else if (info == ELEMENT_INFO_OCCUPIED) {
			if (keyComparer(keys[index], key)) {
				elementFound = true;
				return uint32_t(index);
			}
		}
	}

	return uint32_t(~0);
}

} // namespace sfz
