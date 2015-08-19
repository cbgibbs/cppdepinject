
#include <catch.hpp>

#include "cppdepinject/Repository.h"

using namespace std;
using namespace cppdepinject;

//=================================================================================================

struct A
{
	A() : a() {}
	A( const string& a ) : a( a ) {}

	string a;
};

struct B : public A
{
	B() : A() {}
	B( const string& a ) : A( a ) {}
};

struct C
{
	C( std::shared_ptr<A> a ) : a( a ) {}

	std::shared_ptr<A> a;
};

auto IsA = [] ( const std::string& name ) -> bool { return name == "A"; };
auto StartsA = [] ( const std::string& name ) -> bool { return name.substr( 0, 1 ) == "A"; };


//=================================================================================================

TEST_CASE( "resolveNoArgs_EmptyRepository_ThrowsException" ) {
	Repository r;
	REQUIRE_THROWS_AS( r.resolve<A>(), RepositoryException );
}

TEST_CASE( "resolveNoArgs_HasUnnamedObject_ReturnsObject" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );

	REQUIRE( r.resolve<A>() );
}

TEST_CASE( "resolveNoArgs_HasNamedObject_ReturnsObject" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "A" );

	REQUIRE( r.resolve<A>() );
}

TEST_CASE( "resolveNoArgs_HasTwoObjects_ThrowsException" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );
	r.registerInstance<A>( SimpleCreator<A>(), "A" );

	REQUIRE_THROWS_AS( r.resolve<A>(), RepositoryException );
}

//=================================================================================================

TEST_CASE( "resolveByName_EmptyRepository_ThrowsException" ) {
	Repository r;
	REQUIRE_THROWS_AS( r.resolveByName<A>( "A" ), RepositoryException );
}

TEST_CASE( "resolveByName_HasUnnamedObject_ReturnsObject" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );

	REQUIRE_THROWS_AS( r.resolveByName<A>( "A" ), RepositoryException );
}

TEST_CASE( "resolveByName_HasCorrectNamedObject_ReturnsObject" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "A" );

	REQUIRE( r.resolveByName<A>( "A" ) );
}

TEST_CASE( "resolveByName_HasOtherObject_ThrowsException" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "!A" );

	REQUIRE_THROWS_AS( r.resolveByName<A>( "A" ), RepositoryException );
}

TEST_CASE( "resolveByName_HasTwoObjects_ReturnsCorrectObject" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );
	r.registerInstanceObject<A>( shared_ptr<A>( new A( "a" ) ), "A" );
	r.registerInstanceObject<A>( shared_ptr<A>( new A( "b" ) ), "B" );

	shared_ptr<A> a( r.resolveByName<A>( "A" ) );
	REQUIRE( a );
	REQUIRE( "a" == a->a );
}

//=================================================================================================

TEST_CASE( "resolveFilter_EmptyRepository_ThrowsException" ) {
	Repository r;

	REQUIRE_THROWS_AS( r.resolve<A>( IsA ), RepositoryException );
}

TEST_CASE( "resolveFilter_HasMatchingObject_ReturnsObject" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "A" );

	REQUIRE( r.resolve<A>( IsA ) );
}

TEST_CASE( "resolveFilter_HasNonMatchingObject_ThrowsException" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "!A" );

	REQUIRE_THROWS_AS( r.resolve<A>( IsA ), RepositoryException );
}

TEST_CASE( "resolveFilter_HasTwoObjects_ReturnsCorrectObject" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );
	r.registerInstanceObject<A>( shared_ptr<A>( new A( "a" ) ), "A" );
	r.registerInstanceObject<A>( shared_ptr<A>( new A( "b" ) ), "B" );

	// lambda to filter on registered name
	shared_ptr<A> a( r.resolve<A>( IsA ) );
	REQUIRE( a );
	REQUIRE( "a" == a->a );
}

TEST_CASE( "resolveFilter_HasTwoMatchingObjects_ThrowsException" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "Aa" );
	r.registerInstance<A>( SimpleCreator<A>(), "Ab" );

	REQUIRE_THROWS_AS( r.resolve<A>( StartsA ), RepositoryException );
}

//=================================================================================================

TEST_CASE( "resolveAllFilter_EmptyRepository_ReturnsEmpty" ) {
	Repository r;

	REQUIRE( r.resolveAll<A>( IsA ).empty() );
}

TEST_CASE( "resolveAllFilter_HasMatchingObject_ReturnsObject" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "A" );

	auto found = r.resolveAll<A>( IsA );
	REQUIRE( 1 == found.size() );
}

TEST_CASE( "resolveAllFilter_HasNonMatchingObject_ReturnsEmpty" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>(), "!A" );

	REQUIRE( r.resolveAll<A>( IsA ).empty() );
}

TEST_CASE( "resolveAllFilter_HasTwoObjects_ReturnsCorrectObject" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );
	r.registerInstanceObject<A>( shared_ptr<A>( new A( "a" ) ), "A" );
	r.registerInstanceObject<A>( shared_ptr<A>( new A( "b" ) ), "B" );

	auto found = r.resolveAll<A>( IsA );
	REQUIRE( 1 == found.size() );
	REQUIRE( "a" == found.begin()->second->a );
}

TEST_CASE( "resolveAllFilter_HasTwoMatchingObjects_ReturnsBoth" ) {
	Repository r;
	r.registerInstanceObject<A>( shared_ptr<A>( new A( "aa" ) ), "Aa" );
	r.registerInstanceObject<A>( shared_ptr<A>( new A( "ab" ) ), "Ab" );

	auto found = r.resolveAll<A>( StartsA );
	REQUIRE( 2 == found.size() );
	REQUIRE( "aa" == found.begin()->second->a );
	REQUIRE( "ab" == (++found.begin())->second->a );
}

//=================================================================================================

TEST_CASE( "resolveAllDefault_EmptyRepository_ReturnsEmpty" ) {
	Repository r;

	REQUIRE( r.resolveAll<A>().empty() );
}

TEST_CASE( "resolveAllDefault_HasOneObject_ReturnsObject" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );

	auto found = r.resolveAll<A>();
	REQUIRE( 1 == found.size() );
}

TEST_CASE( "resolveAllDefault_HasTwoObjects_ReturnsBoth" ) {
	Repository r;
	r.registerInstanceObject<A>( shared_ptr<A>( new A( "aa" ) ), "Aa" );
	r.registerInstanceObject<A>( shared_ptr<A>( new A( "ab" ) ), "Ab" );

	auto found = r.resolveAll<A>();
	REQUIRE( 2 == found.size() );
	REQUIRE( "aa" == found.begin()->second->a );
	REQUIRE( "ab" == (++found.begin())->second->a );
}

//=================================================================================================

TEST_CASE( "resolve_ObjectNeedingOtherObjectRegisteredFirst_ReturnsCorrectlyConstructedObject" ) {
	Repository r;
	r.registerInstance<A>( SimpleCreator<A>() );
	r.registerInstance<C>( SimpleCreator_OneResolved<C, A>() );

	auto c = r.resolve<C>();
	REQUIRE( c );
	REQUIRE( c->a );
}

TEST_CASE( "resolve_ObjectNeedingOtherObjectRegisteredAfter_ReturnsCorrectlyConstructedObject" ) {
	Repository r;
	r.registerInstance<C>( SimpleCreator_OneResolved<C, A>() );
	r.registerInstance<A>( SimpleCreator<A>() );

	auto c = r.resolve<C>();
	REQUIRE( c );
	REQUIRE( c->a );
}

//=================================================================================================

TEST_CASE( "resolve_AfterRegisterInstanceObject_ReturnsGivenObjectEachTime" ) {
	Repository r;
	std::shared_ptr<A> a( new A );
	r.registerInstanceObject<A>( a );

	REQUIRE( a == r.resolve<A>() );
	REQUIRE( a == r.resolve<A>() );
}

TEST_CASE( "resolve_AfterRegisterInstance_ReturnsSameObjectEachTime" ) {
	Repository r;
	r.registerInstance<A>( [] ( const Repository& ) { return shared_ptr<A>( new B ); } );

	REQUIRE( r.resolve<A>() == r.resolve<A>() );
}

TEST_CASE( "resolve_AfterRegisterType_ReturnsDifferentObjectEachTime" ) {
	Repository r;
	r.registerType<A>( [] ( const Repository& ) { return shared_ptr<A>( new B ); } );

	REQUIRE( r.resolve<A>() != r.resolve<A>() );
}

//=================================================================================================
