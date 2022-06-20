use yew::prelude::*;
use yew_router::prelude::*;

#[derive(Clone, Routable, PartialEq)]
pub enum Route {
    #[at("/")]
    Home,
    #[at("/signup")]
    SignUp,
    #[at("/create")]
    Create,
    #[not_found]
    #[at("/404")]
    NotFound,
}

#[function_component(NavBar)]
pub fn nav_bar() -> Html {
    html! {
        <ul class="topbar">
            <li><Link<Route> to={Route::Home}><listitem>{ "Home" }</listitem></Link<Route>></li>
            <li><Link<Route> to={Route::SignUp}><listitem>{ "Sign Up" }</listitem></Link<Route>></li>
            <li><Link<Route> to={Route::Create}><listitem>{ "Create" }</listitem></Link<Route>></li>
        </ul>
    }
}