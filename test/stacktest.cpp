#include <stdio.h>

#include "stack.h"


void main()
{
	STACK st(10);

	st.push(1);
	st.push(2);
	st.push(3);
	st.push(4);
	st.push(5);

	st.show();

	st.push(6);
	st.push(7);
	st.push(8);
	st.push(9);
	st.push(10);

	st.show();

	st.push(11);
	st.push(12);

	st.show();
}