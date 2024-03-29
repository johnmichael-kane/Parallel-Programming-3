/*Every present has a unique tag
Minotaur wants to create a chain of presents from unordered bag,
and create a chain of presents through a linked-list.

minotaur asked 4 of his servants to help create the chain of presenets/write cards
each servant does 1/3 things (in no particular order)
1. take a present from the unordered bag, and add it to the chain by 
hooking to predescessor's link (also make sure it's connectred to the next)
2. write a thank you card to the guest and remove the present from the chain
relink the previous one with the next chain in the linked-list
3. Per the minotaur's request, check if a gift with a tag was present in the chain,
without adding/removing a new gift, a serbant scans through and checks if a gift with
that tag is aded to the unordered chain.

minotaur is impatient to get it done, the servants not to wait until all the presents are placed
in the bag of chains of linked and ordered, instead. every, servant was asked to alternate 
adding gifts to the ordered chain and writing "Thank you" cards. Not to stop until everything is done.

they realized at the end of the day there was more presents than thank you notes. what went wrong?
500,000 presents in this scenario, in a concurrent linked-list
*/