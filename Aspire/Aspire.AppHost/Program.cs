var builder = DistributedApplication.CreateBuilder(args);

// Simple approach - let Aspire handle endpoints automatically
var api = builder.AddProject<Projects.API>("api");

var blazor = builder.AddProject<Projects.Blazor>("blazor")
    .WithReference(api);

builder.Build().Run();