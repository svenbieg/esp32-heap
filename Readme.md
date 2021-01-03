<h1>ESP32-heap</h1>

<p>
This heap-component is based on my <a href="http://www.github.com/svenbieg/clusters">Clusters</a> sorting-algorithm.<br />
Free space is mapped by size and by offset, so the smallest free block top most of the heap is returned.<br />
Allocating and freeing 30.000 times takes about 496ms, compared to 746ms with the original heap-component.<br />
</p><br />

<img src="https://user-images.githubusercontent.com/12587394/103431851-2114df80-4bd7-11eb-82fd-5c87cd22f8e0.jpg" />

<br /><br /><br /><br /><br />
