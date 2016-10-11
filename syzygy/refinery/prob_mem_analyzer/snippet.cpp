void foo() {
	TypePtr ptr = bar();
	UserDefinedTypePtr ud_ptr;
	ArrayTypePtr arr_ptr;
	if (ptr->CastTo(&ud_ptr)) {
		// Do stuff with ud_ptr to extract information about the user defined type
	}
	else if (ptr->CastTo(&arr_ptr)) {
		// Get array information
	}
}