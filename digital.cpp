
#include "array_processor.h"
#include "util/time.h"
#include "gs_error.h"
#include "message_row_store.h"
#include "schema.h"
#include "value_operator.h"

/*!
	@brief Compare message field value with object field value
*/
int32_t ArrayProcessor::compare(TransactionContext &txn, ObjectManagerV4 &, AllocateStrategy &,
	ColumnId columnId, MessageRowStore *messageRowStore,
	uint8_t *objectRowField) {
	const uint8_t *inputField;
	uint32_t inputFieldSize;
	messageRowStore->getField(columnId, inputField, inputFieldSize);
	inputField += ValueProcessor::getEncodedVarSize(inputFieldSize);


	int32_t result;
	uint32_t objectRowFieldSize = ValueProcessor::decodeVarSize(objectRowField);
	objectRowField += ValueProcessor::getEncodedVarSize(objectRowFieldSize);

	result = compareBinaryBinary(
		txn, inputField, inputFieldSize, objectRowField, objectRowFieldSize);
	return result;
}

/*!
	@brief Compare object field values
*/
int32_t ArrayProcessor::compare(TransactionContext &txn, ObjectManagerV4 &, AllocateStrategy &,
	ColumnType, uint8_t *srcObjectRowField, uint8_t *targetObjectRowField) {
	int32_t result;
	uint32_t srcObjectRowFieldSize = 0;
	if (srcObjectRowField) {
		srcObjectRowFieldSize =
			ValueProcessor::decodeVarSize(srcObjectRowField);
		srcObjectRowField +=
			ValueProcessor::getEncodedVarSize(srcObjectRowFieldSize);
	}

	uint32_t targetObjectRowFieldSize = 0;
	if (targetObjectRowField) {
		targetObjectRowFieldSize =
			ValueProcessor::decodeVarSize(targetObjectRowField);
		targetObjectRowField +=
			ValueProcessor::getEncodedVarSize(targetObjectRowFieldSize);
	}

	result = compareBinaryBinary(txn, srcObjectRowField, srcObjectRowFieldSize,
		targetObjectRowField, targetObjectRowFieldSize);
	return result;
}

/*!
	@brief Set field value to message
*/
void ArrayProcessor::getField(TransactionContext &, ObjectManagerV4 &, AllocateStrategy &,
	ColumnId columnId, const Value *objectValue, MessageRowStore *messageRowStore) {
	messageRowStore->setArrayField(columnId);
	if (objectValue->data() != NULL) {
		const ArrayObject arrayObject(
			const_cast<uint8_t *>(objectValue->data()));
		const ColumnType elemType =
				messageRowStore->getColumnInfoList()[columnId]
				.getArrayElementType();
		const uint32_t elemSize = FixedSizeOfColumnType[elemType];

		const uint32_t num = arrayObject.getArrayLength();
		const uint32_t totalSize =
				ValueProcessor::getEncodedVarSize(num) + elemSize * num;
		messageRowStore->setVarDataHeaderField(columnId, totalSize);
		messageRowStore->setVarSize(num); 
		for (uint32_t i = 0; i < num; i++) {
			const uint8_t *elemData = arrayObject.getArrayElement(i, elemSize);
			messageRowStore->addArrayElement(elemData, elemSize);
		}
	}
	else { 
		messageRowStore->setVarDataHeaderField(columnId, 1);
		messageRowStore->setVarSize(0); 
	}
}
