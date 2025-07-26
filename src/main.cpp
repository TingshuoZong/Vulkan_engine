#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <Core/Engine.h>
#include <Core/ECS.h>

struct TestComponent {
	int componentNum = 0;
	float param2;

	TestComponent(int componentNum, float param2)
		:componentNum(componentNum), param2(param2){};
	void my_test_method() {
		param2++;
	}
};

int main (int argc, char* argv[]) {
	return init();	// TODO: refactor so main does things but Engine starts things like Renderer
	// EntityManager entityManager;
	// auto& testComponentManager = entityManager.get_component_manager<TestComponent>();
	// Entity firstEntity = entityManager.create_entity();
	// Entity secondEntity = entityManager.create_entity();
	//
	// TestComponent firstEntitysComponent(1, 1.2);
	// TestComponent secondEntitysComponent(2, 3.2);
	//
	// entityManager.get_component_manager<TestComponent>().add_component(firstEntity, firstEntitysComponent);
	// testComponentManager.add_component(secondEntity, secondEntitysComponent);
	//
	// std::cout << "first componentnum -->" << entityManager.get_component_manager<TestComponent>().get_component(firstEntity)->param2 << std::endl;
	// std::cout << "second componentnum -->" << testComponentManager.get_component(secondEntity)->param2 << std::endl;
	//
	// std::cout << "This should be 2 -->" << testComponentManager.get_raw_component_list().size() << std::endl;
	// testComponentManager.remove_component(firstEntity);
	// std::cout << "This should be 1 -->" << testComponentManager.get_raw_component_list().size() << std::endl;
	// std::cout << "second componentnum -->" << testComponentManager.get_component(secondEntity)->param2 << std::endl;
}